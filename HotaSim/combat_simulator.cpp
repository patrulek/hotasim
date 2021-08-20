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
#include <unordered_set>

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
		const auto attack_score = std::max(std::min(current_fight_value / initial_fight_value, 1.0f), 0.0f);

		return static_cast<uint64_t>((1 << 15) * (1.0f - attack_score));
	}

	uint64_t CombatSimulator::evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const {
		const float initial_fight_value = static_cast<float>(calculateBaseHeroFightValue(_initial_state.attacker));
		const float current_fight_value = static_cast<float>(calculateBaseHeroFightValue(_state.attacker));
		const auto defense_score = std::max(std::min(current_fight_value / initial_fight_value, 1.0f), 0.0f);

		return static_cast<uint64_t>((1 << 15) * defense_score);
	}

	uint64_t CombatSimulator::evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const {
		//return static_cast<uint64_t>((1 << 15) * (1.0f - (_state.turn - _initial_state.turn) / 250.0f));

		auto turn_score = static_cast<uint64_t>((1 << 7) * (1.0f - (_state.turn - _initial_state.turn) / 100.0f)) << 8;
		//turn_score = std::min(turn_score, (uint64_t)(1 << 7) << 8);
		auto order_score = 0.5f;
		const auto attacker_score = static_cast<float>(calculateHeroFightValue(_state.attacker));
		const auto defender_score = static_cast<float>(calculateHeroFightValue(_state.defender));

		for (auto& order_unit : _state.order) {
			const auto& unit = manager->getStackByGlobalId(order_unit);
			const auto unit_score = static_cast<float>(calculateStackUnitFightValue(unit));

			if (order_unit < 21)
				order_score += (unit_score / attacker_score) / 2.0f;
			else
				order_score -= (unit_score / defender_score) / 2.0f;
		}

		order_score = std::max(std::min(order_score, 1.0f), 0.0f);
		
		return turn_score | (static_cast<uint64_t>((1 << 7) * order_score));
	}

	uint64_t CombatSimulator::evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const {
		// return (1 << 15);

		const bool has_mana = _initial_state.attacker.getMana();
		uint64_t mana_score = (1 << 7);
		if (has_mana)
			mana_score = static_cast<uint64_t>((1 << 7) * (1.0f - (float)(_state.attacker.getMana() - _initial_state.attacker.getMana()) / _initial_state.attacker.getMana()));
		
		
		std::unordered_set<uint8_t> free_hexes;
		std::unordered_set<uint8_t> reachable_hexes;
		std::unordered_set<uint8_t> ai_reachable_hexes;
		std::unordered_set<uint8_t> attackable_hexes;
		uint64_t team_awareness_score = 0;

		if (_state.last_unit != -1 && _state.last_unit < 21) {
			const auto& last_unit = manager->getStackByGlobalId(_state.last_unit);
			const auto& reachables_ = manager->getCombatAI().getReachableHexesForUnit(last_unit);
			reachable_hexes.insert(std::begin(reachables_), std::end(reachables_));
			free_hexes.insert(std::begin(reachables_), std::end(reachables_));
		}

		for (auto& order_unit : _state.order) {
			if (order_unit >= 21)
				continue;

			const auto& unit = manager->getStackByGlobalId(order_unit);
			const auto& reachables = manager->getCombatAI().getReachableHexesForUnit(unit);
			reachable_hexes.insert(std::begin(reachables), std::end(reachables));
			free_hexes.insert(std::begin(reachables), std::end(reachables));
		}
		
		for (auto& order_unit : _state.order) {
			if (order_unit < 21)
				continue;

			const auto& unit = manager->getStackByGlobalId(order_unit);
			const auto& attackables = manager->getCombatAI().getAttackableHexesForUnit(unit);
			const auto& reachables = manager->getCombatAI().getReachableHexesForUnit(unit);
			attackable_hexes.insert(std::begin(attackables), std::end(attackables));
			ai_reachable_hexes.insert(std::begin(reachables), std::end(reachables));
		}

		for (const auto& att : attackable_hexes)
			free_hexes.erase(att);

		int alive_un = 0;
		for (auto unit : manager->getCurrentState().attacker.getUnitsPtrs()) {
			int score = 0;
			int alive_units = 0;

			for (auto def_unit : manager->getCurrentState().defender.getUnitsPtrs()) {
				if (!def_unit->isAlive())
					continue;
				alive_units++;
				score += manager->getCombatAI().canUnitAttackHex(*unit, def_unit->getHex());
			}

			if (score == alive_units)
				team_awareness_score++;
			alive_un = std::max(alive_un, alive_units);
		}

		const size_t max_free_hexes = FIELD_SIZE - 2 * FIELD_ROWS - _state.field.getOccupied().size();
		uint64_t max_map_awareness_score = max_free_hexes * 100;
		uint64_t map_awareness_score = static_cast<uint64_t>((1 << 3) * 
			((float)(92.5f * free_hexes.size() + 5.0f * reachable_hexes.size() + 2.5f * (max_free_hexes - ai_reachable_hexes.size())) 
			/ max_map_awareness_score)) << 8;

		map_awareness_score = std::min(map_awareness_score, (uint64_t)(1 << 3) << 8);

		team_awareness_score = static_cast<uint64_t>(((float)team_awareness_score / alive_un) * (1 << 3)) << 12;
		team_awareness_score = std::min(team_awareness_score, (uint64_t)(1 << 3) << 12);

		return team_awareness_score | map_awareness_score | mana_score;
	}

	// TODO: make more permutations; for now dont change unit order
	void CombatSimulator::findBestAttackerPermutations() {
		ArmyPermutation permutation;

		for (uint8_t i = 0; i < attacker->army.size(); ++i)
			permutation.permutations.push_back(UnitPermutation{ i, i, attacker->army[i].stack_number });

		permutations.push_back(permutation);
	}

	void CombatSimulator::setDefenderPermutation() {
		for (uint8_t i = 0; i < defender->army.size(); ++i)
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
			for (auto& permutation : permutations) {
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
				tree.addState(manager->getCurrentState(), 0, 1, 0x0000800000000000, 1);
				int action_cnt = 0;
				int seed =  std::random_device()();
				int player_flag = 0;
				int first_turn_actions = 0;
				bool check_first_turn = true;
				//size_t tsize = 0;

				while (!simulatorConstraintsViolated(tree)) {
					int cb_finish_cnt = combat_finished_cnt;
					bool was_player_attack = false;
					bool player_started = false;
					int tidx = tree.current->id;

					while (!combatConstraintsViolated(tree)) {
						auto tsize = tree.getSize();
						if (tsize / 5000 > last_size) {
							++last_size;
							std::cout << "Total states checked: " << std::dec << tree.getSize() << std::endl;
							std::cout << "Total percentage checked: " << (float)tree.level_occurence[1] / first_turn_actions << std::endl;
							std::cout << "Circular occurences: " << tree.circular_occurence << std::endl;
							std::cout << "Early cutoffs: " << tree.early_cutoff << std::endl;
							std::cout << "Forgotten paths/total jumps: " << tree.forgotten_paths.size() - tree.fp_cnt << "/" << jump_random_parent + jump_root << std::endl;
							std::cout << "Estimated turns rule violated: " << turns_rule_violation_cnt << std::endl;
							std::cout << "Combat finished rule violated: " << combat_finished_cnt << std::endl;
							std::cout << "Turns occurence: [0] = " << tree.turns_occurence[0] << " [1] = " << tree.turns_occurence[1]
								<< " [2] = " << tree.turns_occurence[2] << " [3] = " << tree.turns_occurence[3] << " [4] = " << tree.turns_occurence[4]
								<< " [5] = " << tree.turns_occurence[5] << " [6] = " << tree.turns_occurence[6] << " [7] = " << tree.turns_occurence[7]
								<< " [8] = " << tree.turns_occurence[8] << " [9] = " << tree.turns_occurence[9] << " [10] = " << tree.turns_occurence[10] << std::endl;
							std::cout << "Depth occurence: [0] = " << tree.level_occurence[0] << " [1] = " << tree.level_occurence[1]
								<< " [2] = " << tree.level_occurence[2] << " [3] = " << tree.level_occurence[3] << " [4] = " << tree.level_occurence[4]
								<< " [5] = " << tree.level_occurence[5] << " [6] = " << tree.level_occurence[6] << " [7] = " << tree.level_occurence[7]
								<< " [8] = " << tree.level_occurence[8] << " [9] = " << tree.level_occurence[9] << " [10] = " << tree.level_occurence[10] << std::endl;
							std::cout << "Cache access/miss: " << std::dec << CombatPathfinder::cache_access << " / " << CombatPathfinder::cache_misses << std::endl;
							std::cout << "Best score so far: " << std::hex << tree.root->best_branch_score << std::dec << std::endl << std::endl;
						}
						/*if (tsize == 980991) {
							__debugbreak();
						}*/
						if (manager->isUnitMove()) {
							if (manager->isPlayerMove()) {

								auto actions = manager->generateActionsForPlayer();
								//auto action_order = Estimator::shuffleActions(actions, *manager, seed);
								
								auto action_idx = action_cnt;// action_order[action_cnt];

								if( first_turn_actions == 0)
									first_turn_actions = actions.size();
								//std::cout << "Player action (" << action_idx + 1 << " / " << actions.size() << "): " << (int)actions[action_idx].action
								//	<< " - " << actions[action_idx].target << " ### Unit: " << manager->getActiveStack().getGlobalUnitId() << "\n";
								//const_cast<CombatPathfinder&>(manager->getCombatAI().getPathfinder()).storePathCache(false);
								for (int i = 0; i < actions.size(); ++i) {
									manager->nextStateByAction(actions[i]);
									tree.addState(manager->getCurrentState(), i, (int)actions.size(),
										evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), seed);

									if( !tree.circular_path_found && i != actions.size() - 1)
										tree.goParent();

									if (i != actions.size() - 1) {
										serializer->unpackCombatState(*tree.current->state);
										manager->setCurrentState(*tree.current->state);
										manager->getCurrentState().field.rehash();
										//const_cast<CombatPathfinder&>(manager->getCombatAI().getPathfinder()).restorePathCache();
									}
								}

								if (!tree.node_order.empty()) {
									tree.takeBest(true);
									serializer->unpackCombatState(*tree.current->state);
									manager->setCurrentState(*tree.current->state);
									manager->getCurrentState().field.rehash();
								}
								//const_cast<CombatPathfinder&>(manager->getCombatAI().getPathfinder()).restorePathCache();

								//seed = action_cnt % 40 > 10 ? std::random_device()() : 42;
								//seed = 42;
								action_cnt = 0;
							}
							else {
								auto actions = manager->generateActionsForAI();
								// TODO: for now, take only first ai action (in most cases there will be one action anyway)
								manager->nextStateByAction(actions[0]);
								tree.addState(manager->getCurrentState(), 0, 1, 
									evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1, false);
								//std::cout << "AI Action (size = " << actions.size() << ")\n";
							}
							continue;
						}

						manager->nextState();
						tree.addState(manager->getCurrentState(), 0, 1,
							evaluateCombatStateScore(manager->getInitialState(), manager->getCurrentState()), 1);

						//std::cout << "\n--- Start Turn " << manager->getCurrentState().turn << " --- \n\n";
					}


					

					bool random_jump = false;
					bool root_jump = false;
					bool take_forgotten = false;

					// check if need to go up further
					//if (tree.foundCircularPath()) {
					//	//std::cout << " CIRCULAR PATH FOUND\n\n";
					//	if (tree.unitStackLastAction()) {
					//		//tree.goParent();
					//		while (tree.hasParent() && tree.unitStackLastAction())
					//			tree.goParent();

					//		action_cnt = tree.current->action + 1;
					//		tree.goParent();
					//	}
					//	else {
					//		// if we found circular path, and it isnt last unit of action, just go back one more state
					//		action_cnt = tree.current->action + 1;
					//		tree.goParent();
					//	}
					//}
					//else if (!root_jump && !random_jump) {
					//	//std::cout << " LAST ACTION, JUMP\n\n";
					//	while (tree.hasParent() && tree.unitStackLastAction())
					//		tree.goParent();

					//	// wasnt last action of this unit, so we need to go back one more state (before that unit acted)
					//	action_cnt = tree.current->action + 1;
					//	tree.goParent();
					//	//tree.goParent();
					//	/*while (tree.current->parent && tree.current->action + 1 >= tree.current->action_size)
					//		tree.goParent();

					//	if (tree.current->parent)
					//		tree.goParent();*/
					//}
					//if (action_cnt == tree.current->children.back()->action_size) {
					if (tree.node_order.empty()) {
						std::cout << "All states reached. End simulation\n";
						break;
					}
					tree.takeBest();

					//std::cout << " --- CURRENT TURN: " << tree.current->state->turn << " --- \n\n";
					//size_t xsize = tree.getSize();
					seed = tree.current->seed;
					serializer->unpackCombatState(*tree.current->state);
					manager->setCurrentState(*tree.current->state);
					manager->getCurrentState().field.rehash();

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
					<< " [5] = " << tree.turns_occurence[5] << " [6] = " << tree.turns_occurence[6] << " [7] = " << tree.turns_occurence[7]
					<< " [8] = " << tree.turns_occurence[8] << " [9] = " << tree.turns_occurence[9] << " [10] = " << tree.turns_occurence[10] << std::endl;
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