#include "combat_simulator.h"

#include "../HotaMechanics/utils.h"

#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_state.h"
#include "../HotaMechanics/combat_calculator.h"
#include "combat_sequencetree.h"
#include "combat_estimator.h"
#include "combat_rewinder.h"

#include <list>
#include <iostream>
#include <random>

using namespace HotaMechanics;
using namespace HotaMechanics::Calculator;
using namespace HotaMechanics::Utils;

namespace HotaSim {
	CombatSimulator::CombatSimulator(const Hero& _attacker, const Hero& _defender,
		const CombatFieldType _field_type, const CombatType _combat_type)
		: field_type(_field_type), combat_type(_combat_type) {

		attacker = std::make_unique<Hero>(_attacker);
		defender = std::make_unique<Hero>(_defender);
	}

	CombatSimulator::~CombatSimulator() {}

	void CombatSimulator::updateBestState(const CombatState& _state, const std::list<CombatState>& _states, const std::list<CombatAction>& _actions) {
		if (evaluateCombatStateScore(manager->getInitialState(), _state) <= evaluateCombatStateScore(manager->getInitialState(), *best_state))
			return;

		best_state = std::make_unique<CombatState>(_state);
		states_timeline = _states;
		actions_timeline = _actions;
	}

	void CombatSimulator::setCombatManager(const CombatManager& _mgr) {
		manager = std::make_unique<CombatManager>(_mgr.getAttacker(), _mgr.getDefender(), _mgr.getInitialCombatField(), _mgr.getCombatType());
	}

	uint64_t CombatSimulator::evaluateCombatStateScore(const CombatState& _initial_state, const CombatState& _state) const {
		uint64_t attack_score = evaluateCombatStateAttackScore(_initial_state, _state); // the more enemy units we killed the better
		uint64_t defense_score = evaluateCombatStateDefenseScore(_initial_state, _state); // the less units we lost the better
		uint64_t turns_score = evaluateCombatStateTurnsScore(_initial_state, _state); // the less turns we fought the better
		uint64_t mana_score = evaluateCombatStateManaScore(_initial_state, _state); // the less mana weve lost the better

		uint64_t score = (attack_score << 48) | (defense_score << 32) | (turns_score << 16) | mana_score;
		return score;
	}

	uint64_t CombatSimulator::evaluateCombatStateAttackScore(const CombatState& _initial_state, const CombatState& _state) const {
		float initial_fight_value = calculateBaseHeroFightValue(_initial_state.defender);
		float current_fight_value = calculateBaseHeroFightValue(_state.defender);

		return (1 << 15) * (1.0f - current_fight_value / initial_fight_value);
	}

	uint64_t CombatSimulator::evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const {
		float initial_fight_value = calculateBaseHeroFightValue(_initial_state.attacker);
		float current_fight_value = calculateBaseHeroFightValue(_state.attacker);

		return (1 << 15) * (current_fight_value / initial_fight_value);
	}

	uint64_t CombatSimulator::evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const {
		return (1 << 15) * (1.0f - (_state.turn - _initial_state.turn) / 250.0f);
	}

	uint64_t CombatSimulator::evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const {
		bool has_mana = _initial_state.attacker.getMana();

		if (has_mana)
			return (1 << 15) * (1.0f - (_state.attacker.getMana() - _initial_state.attacker.getMana()) / _initial_state.attacker.getMana());
		return (1 << 15);
	}

	// TODO: make more permutations; for now dont change unit order
	void CombatSimulator::findBestAttackerPermutations() {
		ArmyPermutation permutation;

		for (int8_t i = 0; i < attacker->army.size(); ++i)
			permutation.permutations.push_back(UnitPermutation{ i, i, attacker->army[i].stack_number });

		permutations.push_back(permutation);
	}

	void CombatSimulator::setDefenderPermutation() {
		for (int8_t i = 0; i < defender->army.size(); ++i)
			defender_permutation.permutations.push_back(UnitPermutation{ i, i, defender->army[i].stack_number });
	}

	const bool CombatSimulator::simulatorConstraintsViolated(const CombatSequenceTree& _tree) {
		// some states wasnt checked yet
		bool some_states_left_rule_violated = _tree.isCurrentRoot() && !_tree.canTakeForgotten();

		// reached total states limit for simulation
		bool state_limit_reached_rule_violated = _tree.getSize() > estimated_total_states;

		if (_tree.isCurrentRoot() && !_tree.forgotten_paths.empty())
			const_cast<CombatSequenceTree&>(_tree).takeForgotten();

		return some_states_left_rule_violated || state_limit_reached_rule_violated;
	}

	const bool CombatSimulator::combatConstraintsViolated() {
		// turns rule
		bool turns_rule_violated = manager->getCurrentState().turn >= estimated_turns;
		turns_rule_violation_cnt += turns_rule_violated;

		// combat finished rule
		bool combat_finished_rule_violated = manager->isCombatFinished();
		combat_finished_cnt += combat_finished_rule_violated;

		return turns_rule_violated || combat_finished_rule_violated;
	}

	void CombatSimulator::resetRulesCounters() {
		combat_finished_cnt = turns_rule_violation_cnt = 0;
	}

	void CombatSimulator::start() {
		for (int i = 0; i < 1 /* combat field templates */; ++i) {
			for (auto permutation : permutations) {
				prepareCombat(permutation, /*i*/ CombatFieldTemplate::IMPS_2x100);

				CombatSequenceTree tree(manager->getInitialState());
				int last_size = 0;
				int jump_root = 0;
				int jump_random_parent = 0;

				estimated_turns = Estimator::estimateTurnsNumber(manager->getInitialState());
				estimated_total_states = Estimator::estimateTotalStatesNumber(manager->getInitialState());
				resetRulesCounters();

				// start battle (PRE_BATTLE action)
				manager->nextState();
				tree.addState(manager->getCurrentState(), 0, 1, 0x0000800080008000, 1);
				int action_cnt = 0;
				int seed = std::random_device()();

				while (!simulatorConstraintsViolated(tree)) {
					int cb_finish_cnt = combat_finished_cnt;

					while (!combatConstraintsViolated()) {
						if (manager->isUnitMove()) {
							if (manager->isPlayerMove()) {
								auto actions = manager->generateActionsForPlayer();
								auto action_order = Estimator::shuffleActions(actions, *manager, seed);
								auto action_idx = action_order[action_cnt];
								manager->nextStateByAction(actions[action_idx]);
								tree.addState(manager->getCurrentState(), action_cnt, actions.size(), 
									evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), seed);
								seed = action_cnt % 40 > 10 ? std::random_device()() : 42;
								//seed = 42;
								action_cnt = 0;
							}
							else {
								auto actions = manager->generateActionsForAI();
								// TODO: for now, take only first ai action (in most cases there will be one action anyway)
								manager->nextStateByAction(actions[0]);
								tree.addState(manager->getCurrentState(), 0, 1, 
									evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1);
							}
							continue;
						}

						manager->nextState();
						tree.addState(manager->getCurrentState(), 0, 1,
							evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1);
					}

					if (tree.getSize() / 5000 > last_size) {
						++last_size;
						std::cout << "Total states checked: " << std::dec << tree.getSize() << std::endl;
						std::cout << "Forgotten paths/total jumps: " << tree.forgotten_paths.size() - tree.fp_cnt << std::endl;
						std::cout << "Estimated turns rule violated: " << turns_rule_violation_cnt << std::endl;
						std::cout << "Combat finished rule violated: " << combat_finished_cnt << std::endl;
						std::cout << "Turns occurence: [0] = " << tree.turns_occurence[0] << " [1] = " << tree.turns_occurence[1]
							<< " [2] = " << tree.turns_occurence[2] << " [3] = " << tree.turns_occurence[3] << " [4] = " << tree.turns_occurence[4]
							<< " [5] = " << tree.turns_occurence[5] << " [6] = " << tree.turns_occurence[6] << " [7] = " << tree.turns_occurence[7] << std::endl;
						std::cout << "Cache access/miss: " << std::dec << CombatPathfinder::cache_access << " / " << CombatPathfinder::cache_misses << std::endl;
						std::cout << "Best score so far: " << std::hex << tree.root->best_branch_score << std::endl << std::endl;
					}

					bool random_jump = false;
					bool root_jump = false;
					bool take_forgotten = false;

					if ((float)tree.forgotten_paths.size() / tree.getSize() > 0.01f 
					&& tree.getSize() % 12 == 0)
						take_forgotten = true;

					/*if (tree.getSize() < 45000) {
						++jump_root;
						tree.goRoot(cb_finish_cnt != combat_finished_cnt);
						root_jump = true;
					}
					else */if ((float)tree.forgotten_paths.size() / tree.getSize() < 0.015f) {
						if (tree.getSize() / 3 > jump_random_parent) {
							++jump_random_parent;
							tree.goRandomParent(cb_finish_cnt != combat_finished_cnt);
							random_jump = true;
						}
						else if (tree.getSize() / 7 > jump_root) {
							++jump_root;
							tree.goRoot(cb_finish_cnt != combat_finished_cnt);
							root_jump = true;
						}
					}

					// check if need to go up further
					if (!root_jump) {
						if(!random_jump)
							tree.goParent();

						while (tree.current->parent && tree.current->action + 1 >= tree.current->action_size)
							tree.goParent();

						if (tree.current->parent)
							tree.goParent();
					}

					if (tree.current->parent == tree.root.get() && take_forgotten) {
						tree.takeForgotten();
					}

					action_cnt = tree.current->action + 1;
					seed = tree.current->seed;
					manager->setCurrentState(tree.current->state);
				}

				auto best_leaf = tree.findBestLeaf();
				int best_leaf_turns = best_leaf->state.turn;
				std::vector<int> action_order;
				std::vector<int> action_seeds;
				while (best_leaf) {
					action_order.push_back(best_leaf->action);
					action_seeds.push_back(best_leaf->seed);
					best_leaf = best_leaf->parent;
				}

				action_order.pop_back();
				action_seeds.pop_back();
				std::reverse(std::begin(action_order), std::end(action_order));
				std::reverse(std::begin(action_seeds), std::end(action_seeds));

				std::cout << "Total actions: " << std::dec << action_order.size() << std::endl;
				std::cout << "Total turns: " << best_leaf_turns + 1 << std::endl;
				std::cout << "Total states checked: " << tree.getSize() << std::endl;
				std::cout << "Best result: " << std::hex << tree.root->best_branch_score << std::endl;

				auto rewinder = CombatRewinder(*manager);
				rewinder.rewind(action_order, action_seeds);
			}
		}
	}

	void CombatSimulator::prepareCombat(const ArmyPermutation& permutation, const CombatFieldTemplate _field_template) {
		auto combat_attacker = prepareCombatHero(*attacker, permutation, CombatSide::ATTACKER);
		auto combat_defender = prepareCombatHero(*defender, defender_permutation, CombatSide::DEFENDER);
		auto combat_field = prepareCombatField(_field_template);

		manager = std::make_unique<CombatManager>(*combat_attacker, *combat_defender, *combat_field, combat_type);
		manager->initialize();
	}

	std::shared_ptr<CombatField> CombatSimulator::prepareCombatField(const CombatFieldTemplate _field_template) {
		auto combat_field = std::make_shared<CombatField>(field_type);
		auto combat_field_template = getCombatFieldTemplate(_field_template);
		combat_field->setTemplate(combat_field_template);
		return combat_field;
	}

	std::shared_ptr<CombatHero> CombatSimulator::prepareCombatHero(const Hero& hero_template, const ArmyPermutation& permutation, const CombatSide _side) {
		std::shared_ptr<CombatHero> hero = std::make_shared<CombatHero>(hero_template, permutation, _side);

		return hero;
	}

	void CombatSimulator::initialize() {
		findBestAttackerPermutations();
		setDefenderPermutation();
	}
}; // HotaSim