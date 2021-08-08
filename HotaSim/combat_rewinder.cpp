#include "combat_rewinder.h"

#include "../HotaMechanics/utils.h"
#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_manager.h"

#include "combat_estimator.h"

using namespace HotaMechanics;
using namespace HotaMechanics::Constants;
using namespace HotaMechanics::Utils;

#include <iostream>

namespace HotaSim {

	void CombatRewinder::describeAction(const CombatAction& _action) {
		auto& active_stack = manager.getActiveStack();

		if (_action.action == CombatActionType::WAIT) {
			std::cout << "Unit (" << to_string(active_stack) << ") has waited\n";
		}
		else if (_action.action == CombatActionType::DEFENSE) {
			std::cout << "Unit (" << to_string(active_stack) << ") has defended\n";
		}
		else if (_action.action == CombatActionType::WALK) {
			auto hex = manager.getCurrentState().field.getById(active_stack.getHex());
			auto path = const_cast<CombatPathfinder&>(manager.getCombatAI().getPathfinder()).findPath(active_stack.getHex(), _action.target,
				manager.getCurrentState().field, false);

			int walk_distance = _action.param1 == -1 ? active_stack.getCombatStats().spd : _action.param1;
			walk_distance = std::min(path.size(), (size_t)walk_distance) - 1;
			auto target_hex = manager.getCurrentState().field.getById(path[walk_distance]);

			std::cout << "Unit (" << to_string(active_stack) << ") has walked from " << to_string(hex) << " to " << to_string(target_hex) << "\n";
		}
		else if (_action.action == CombatActionType::ATTACK) {
			auto& defender = manager.getStackByLocalId(_action.param1, active_stack.getEnemyCombatSide());
			auto hex = manager.getCurrentState().field.getById(_action.target);
			std::cout << "Unit (" << to_string(active_stack) << ") chose to attack unit (" << to_string(defender) << ") at " << to_string(hex) << "\n";
		}
	}

	void CombatRewinder::rewind(const std::vector<int>& _actions, const std::vector<int>& _seeds) {
		manager.reinitialize();
		manager.nextState();
		std::cout << "\n\n====================================================\nBattle has started\n===================================================\n\n";
		std::cout << "\nTurn " << manager.getCurrentState().turn + 1 << " has started\n-----\n";

		for (int i = 1; i < _actions.size(); ++i) {
			int action_cnt = _actions[i];

			if (manager.isUnitMove()) {
				if (manager.isPlayerMove()) {
					auto actions = manager.generateActionsForPlayer();

					auto action_order = Estimator::shuffleActions(actions, manager, _seeds[i]);
					auto action_idx = action_order[action_cnt];
					describeAction(actions[action_idx]);
					manager.nextStateByAction(actions[action_idx]);
				}
				else {
					auto actions = manager.generateActionsForAI();
					describeAction(actions[action_cnt]);
					manager.nextStateByAction(actions[action_cnt]);
				}
				continue;
			}

			manager.nextState();
			std::cout << "\nTurn " << manager.getCurrentState().turn + 1 << " has started\n-----\n";
		}

		std::cout << "\nBattle has finished with result: " <<
			(manager.getCurrentState().result == CombatResult::PLAYER ? "player_won" :
				manager.getCurrentState().result == CombatResult::ENEMY ? "ai_won" :
				manager.getCurrentState().result == CombatResult::DRAW ? "both sides died" : "not finished (still in progress)");
		std::cout << "\n\n============================================================================ \n\n";
	}
}