#include "combat_simulator.h"

#include "../HotaMechanics/utils.h"

#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_state.h"
#include "../HotaMechanics/combat_calculator.h"
#include "combat_manager_serializer.h"
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
		const float initial_fight_value = static_cast<float>(calculateBaseHeroFightValue(_initial_state.defender));
		const float current_fight_value = static_cast<float>(calculateBaseHeroFightValue(_state.defender));

		return static_cast<uint64_t>((1 << 15) * (1.0f - current_fight_value / initial_fight_value));
	}

	uint64_t CombatSimulator::evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const {
		const float initial_fight_value = static_cast<float>(calculateBaseHeroFightValue(_initial_state.attacker));
		const float current_fight_value = static_cast<float>(calculateBaseHeroFightValue(_state.attacker));

		return static_cast<uint64_t>((1 << 15) * (current_fight_value / initial_fight_value));
	}

	uint64_t CombatSimulator::evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const {
		return static_cast<uint64_t>((1 << 15) * (1.0f - (_state.turn - _initial_state.turn) / 250.0f));
	}

	uint64_t CombatSimulator::evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const {
		const bool has_mana = _initial_state.attacker.getMana();

		if (has_mana)
			return static_cast<uint64_t>((1 << 15) * (1.0f - (_state.attacker.getMana() - _initial_state.attacker.getMana()) / _initial_state.attacker.getMana()));
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

		//if (_tree.isCurrentRoot() && !_tree.forgotten_paths.empty())
		//	const_cast<CombatSequenceTree&>(_tree).takeForgotten();

		return some_states_left_rule_violated || state_limit_reached_rule_violated;
	}

	const bool CombatSimulator::combatConstraintsViolated(const CombatSequenceTree& _tree) {
		// turns rule
		bool turns_rule_violated = manager->getCurrentState().turn >= estimated_turns;
		turns_rule_violation_cnt += turns_rule_violated;

		// combat finished rule
		bool combat_finished_rule_violated = manager->isCombatFinished();
		combat_finished_cnt += combat_finished_rule_violated;

		// circular path
		bool circular_path_rule_violated = const_cast<CombatSequenceTree&>(_tree).circular_path_found;

		return turns_rule_violated || combat_finished_rule_violated || circular_path_rule_violated;
	}

	void CombatSimulator::resetRulesCounters() {
		combat_finished_cnt = turns_rule_violation_cnt = 0;
	}

	void CombatSimulator::start() {
		for (int i = 0; i < 1 /* combat field templates */; ++i) {
			for (auto permutation : permutations) {
				prepareCombat(permutation, /*i*/ CombatFieldTemplate::IMPS_2x100);

				CombatSequenceTree tree(*serializer, *manager, manager->getInitialState());
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
				int seed =  std::random_device()();

				while (!simulatorConstraintsViolated(tree)) {
					int cb_finish_cnt = combat_finished_cnt;
					bool was_player_attack = false;
					bool player_started = false;

					while (!combatConstraintsViolated(tree)) {
						auto tsize = tree.getSize();
						if (manager->isUnitMove()) {
							if (manager->isPlayerMove()) {
								player_started = true;

								auto actions = manager->generateActionsForPlayer();
								//auto action_order = Estimator::shuffleActions(actions, *manager, seed);
								
								auto action_idx = action_cnt;// action_order[action_cnt];

								if (actions[action_idx].action == CombatActionType::ATTACK)
									was_player_attack = true;

								//std::cout << "Player action (" << action_idx + 1 << " / " << actions.size() << "): " << (int)actions[action_idx].action
								//	<< " - " << actions[action_idx].target << " ### Unit: " << manager->getActiveStack().getGlobalUnitId() << "\n";

								manager->nextStateByAction(actions[action_idx]);
								tree.addState(manager->getCurrentState(), action_cnt, (int)actions.size(), 
									evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), seed);

								//seed = action_cnt % 40 > 10 ? std::random_device()() : 42;
								//seed = 42;
								action_cnt = 0;
							}
							else {
								auto actions = manager->generateActionsForAI();
								// TODO: for now, take only first ai action (in most cases there will be one action anyway)
								bool ai_attack_first = false;
								if (!was_player_attack && player_started && actions[0].action == CombatActionType::ATTACK)
									ai_attack_first = true;

								manager->nextStateByAction(actions[0]);
								tree.addState(manager->getCurrentState(), 0, 1, 
									evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1, ai_attack_first);
								//std::cout << "AI Action (size = " << actions.size() << ")\n";
							}
							continue;
						}

						manager->nextState();
						tree.addState(manager->getCurrentState(), 0, 1,
							evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1);

						player_started = false;
						was_player_attack = false;
						//std::cout << "\n--- Start Turn " << manager->getCurrentState().turn << " --- \n\n";
					}

					if (tree.getSize() / 5000 > last_size) {
						++last_size;
						std::cout << "Total states checked: " << std::dec << tree.getSize() << std::endl;
						std::cout << "Circular occurences: " << tree.circular_occurence << std::endl;
						std::cout << "Early cutoffs: " << tree.early_cutoff << std::endl;
						std::cout << "Forgotten paths/total jumps: " << tree.forgotten_paths.size() - tree.fp_cnt << "/" << jump_random_parent + jump_root << std::endl;
						std::cout << "Estimated turns rule violated: " << turns_rule_violation_cnt << std::endl;
						std::cout << "Combat finished rule violated: " << combat_finished_cnt << std::endl;
						std::cout << "Turns occurence: [0] = " << tree.turns_occurence[0] << " [1] = " << tree.turns_occurence[1]
							<< " [2] = " << tree.turns_occurence[2] << " [3] = " << tree.turns_occurence[3] << " [4] = " << tree.turns_occurence[4]
							<< " [5] = " << tree.turns_occurence[5] << " [6] = " << tree.turns_occurence[6] << " [7] = " << tree.turns_occurence[7] << std::endl;
						std::cout << "Depth occurence: [0] = " << tree.level_occurence[0] << " [1] = " << tree.level_occurence[1]
							<< " [2] = " << tree.level_occurence[2] << " [3] = " << tree.level_occurence[3] << " [4] = " << tree.level_occurence[4]
							<< " [5] = " << tree.level_occurence[5] << " [6] = " << tree.level_occurence[6] << " [7] = " << tree.level_occurence[7]
							<< " [8] = " << tree.level_occurence[8] << " [9] = " << tree.level_occurence[9] << " [10] = " << tree.level_occurence[10] << std::endl;
						std::cout << "Cache access/miss: " << std::dec << CombatPathfinder::cache_access << " / " << CombatPathfinder::cache_misses << std::endl;
						std::cout << "Best score so far: " << std::hex << tree.root->best_branch_score << std::dec << std::endl << std::endl;
					}

					bool random_jump = false;
					bool root_jump = false;
					bool take_forgotten = false;

					// check if need to go up further
					if (tree.foundCircularPath()) {
						//std::cout << " CIRCULAR PATH FOUND\n\n";
						if (tree.unitStackLastAction()) {
							//tree.goParent();
							while (tree.hasParent() && tree.unitStackLastAction())
								tree.goParent();

							action_cnt = tree.current->action + 1;
							tree.goParent();
						}
						else {
							// if we found circular path, and it isnt last unit of action, just go back one more state
							action_cnt = tree.current->action + 1;
							tree.goParent();
						}
					}
					else if (!root_jump && !random_jump) {
						//std::cout << " LAST ACTION, JUMP\n\n";
						while (tree.hasParent() && tree.unitStackLastAction())
							tree.goParent();

						// wasnt last action of this unit, so we need to go back one more state (before that unit acted)
						action_cnt = tree.current->action + 1;
						tree.goParent();
						//tree.goParent();
						/*while (tree.current->parent && tree.current->action + 1 >= tree.current->action_size)
							tree.goParent();

						if (tree.current->parent)
							tree.goParent();*/
					}

					//std::cout << " --- CURRENT TURN: " << tree.current->state->turn << " --- \n\n";
					seed = tree.current->seed;
					serializer->unpackCombatState(*tree.current->state);
					manager->setCurrentState(*tree.current->state);

					if (action_cnt == tree.current->children.back()->action_size) {
						std::cout << "All states reached. End simulation\n";
						break;
					}
				}

				auto best_leaf = tree.findBestLeaf();
				int best_leaf_turns = best_leaf->state->turn;
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
				std::cout << "Actions in order: ";
				for (auto act : action_order)
					std::cout << act << ",";
				std::cout << std::endl;

				std::cout << "Total states checked: " << std::dec << tree.getSize() << std::endl;
				std::cout << "Circular occurences: " << tree.circular_occurence << std::endl;
				std::cout << "Early cutoffs: " << tree.early_cutoff << std::endl;
				std::cout << "Forgotten paths/total jumps: " << tree.forgotten_paths.size() - tree.fp_cnt << "/" << jump_random_parent + jump_root << std::endl;
				std::cout << "Estimated turns rule violated: " << turns_rule_violation_cnt << std::endl;
				std::cout << "Combat finished rule violated: " << combat_finished_cnt << std::endl;
				std::cout << "Turns occurence: [0] = " << tree.turns_occurence[0] << " [1] = " << tree.turns_occurence[1]
					<< " [2] = " << tree.turns_occurence[2] << " [3] = " << tree.turns_occurence[3] << " [4] = " << tree.turns_occurence[4]
					<< " [5] = " << tree.turns_occurence[5] << " [6] = " << tree.turns_occurence[6] << " [7] = " << tree.turns_occurence[7] << std::endl;
				std::cout << "Level occurence: [0] = " << tree.level_occurence[0] << " [1] = " << tree.level_occurence[1]
					<< " [2] = " << tree.level_occurence[2] << " [3] = " << tree.level_occurence[3] << " [4] = " << tree.level_occurence[4]
					<< " [5] = " << tree.level_occurence[5] << " [6] = " << tree.level_occurence[6] << " [7] = " << tree.level_occurence[7]
					<< " [8] = " << tree.level_occurence[8] << " [9] = " << tree.level_occurence[9] << " [10] = " << tree.level_occurence[10] << std::endl;
				std::cout << "Cache access/miss: " << std::dec << CombatPathfinder::cache_access << " / " << CombatPathfinder::cache_misses << std::endl;
				std::cout << "Best score so far: " << std::hex << tree.root->best_branch_score << std::endl << std::endl;
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
		serializer = std::make_unique<CombatSerializer>(*manager);
	}

	std::shared_ptr<CombatField> CombatSimulator::prepareCombatField(const CombatFieldTemplate _field_template) {
		auto combat_field = std::make_shared<CombatField>(field_type, _field_template);
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