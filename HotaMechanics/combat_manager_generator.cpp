#include "combat_manager.h"


#include "combat_unit.h"
#include "combat_ai.h"
#include "combat_pathfinder.h"

namespace HotaMechanics {
	using namespace Constants;

	const std::vector<CombatAction> CombatManager::generateActionsForPlayer() {
		auto& activeStack = getActiveStack();

		if (!activeStack.canMakeAction())
			return std::vector<CombatAction>{};

		std::vector<CombatAction> actions{};
		ai->processEvents();

		if (activeStack.canDefend())
			actions.push_back(createDefendAction());

		if (activeStack.canWait())
			actions.push_back(createWaitAction());

		// get reachable hexes;
		auto reachable_hexes = ai->getReachableHexesForUnit(activeStack);

		//auto& field = current_state->field;
		//auto reachable_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getReachableHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd, field, false, false);
		
		for (auto hex : reachable_hexes) {
			// TODO: to nie powinno byæ potrzebne
			if (!current_state->field.isHexWalkable(hex))
				continue;

			actions.push_back(createWalkAction(hex));
		}

		// get attackable enemy units; 
		// if can shoot then only get all enemy units
		//auto range_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd + 1);
		auto units_in_range = ai->getEnemyUnitsInRange(activeStack);//getUnitsInRange(CombatSide::DEFENDER, range_hexes);

		for (auto unit : units_in_range) {
			auto adjacent_hexes = ai->getPathfinder().getAdjacentHexes(unit->getHex());
			auto adjacent_hexes_vec = std::vector<int16_t>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
			auto reachable_adjacent = ai->getReachableHexesForUnitFromList(activeStack, adjacent_hexes_vec);

			// if were staying in one of adjacent hexes to target, add that hex
			bool staying_around = std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), activeStack.getHex()) != std::end(adjacent_hexes);
			if (staying_around)
				reachable_adjacent.push_back(activeStack.getHex());

			for (auto hex : reachable_adjacent) {
				actions.push_back(createAttackAction(unit->getUnitId(), hex));
			}
		}

		// get castable spells;
		if (activeStack.canCast())
			throw std::exception("Not implemented yet");

		if (activeStack.canHeroCast())
			throw std::exception("Not implemented yet");

		return actions;
	}


	const std::vector<CombatAction> CombatManager::generateActionsForAI() {
		auto& active_stack = getActiveStack();

		if (!active_stack.canMakeAction())
			return std::vector<CombatAction>{};

		ai->processEvents();
		ai->calculateFightValueAdvantageOnHexes(active_stack, current_state->attacker, current_state->field);

		std::vector<CombatAction> actions{};
		auto units_to_attack = ai->chooseUnitToAttack(active_stack, current_state->attacker);

		for (auto unit_id : units_to_attack) {
			auto unit = current_state->attacker.getUnits()[unit_id];
			int hex = ai->chooseHexToMoveForAttack(active_stack, *unit);

			int distance_to_hex = const_cast<CombatPathfinder&>(ai->getPathfinder()).findPath(active_stack.getHex(), hex, current_state->field).size();
			int turns = distance_to_hex == 0 ? 1 : std::ceil((float)distance_to_hex / active_stack.getCombatStats().spd);

			if (turns == 1)
				actions.push_back(createAttackAction(unit_id, hex));
			else {
				int walk_distance = ai->chooseWalkDistanceFromPath(active_stack, hex, current_state->field);

				if (walk_distance == 0) {
					if (active_stack.canWait())
						actions.push_back(createWaitAction());
					else
						actions.push_back(createWalkAction(hex));
				}
				else {
					actions.push_back(createWalkAction(hex, walk_distance));
				}
			}
		}

		return actions;
	}

}; // HotaMechanics