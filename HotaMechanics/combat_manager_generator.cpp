#include "combat_manager.h"


#include "combat_unit.h"
#include "combat_ai.h"
#include "combat_pathfinder.h"

namespace HotaMechanics {
	using namespace Constants;

	const std::vector<CombatAction> CombatManager::generateActionsForPlayer() {
		auto& active_stack = getActiveStack();
		const_cast<CombatPathfinder&>(ai->getPathfinder()).pathMap(active_stack.getHex(), current_state->field, false, active_stack.getCombatStats().spd);

		if (!active_stack.canMakeAction())
			return std::vector<CombatAction>();

		actions.clear();

		// get attackable enemy units; 
		// if can shoot then only get all enemy units
		//auto range_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd + 1);
		auto& units_in_range = ai->getEnemyUnitsInAttackRange(active_stack);//getUnitsInRange(CombatSide::DEFENDER, range_hexes);

		for (auto unit : units_in_range) {
			auto& adjacent = ai->getPathfinder().getAdjacentHexes(unit->getHex());

			for (uint8_t adj_hex = 0; adj_hex < 6; ++adj_hex) {
				if (ai->canUnitAttackHex(active_stack, adjacent[adj_hex]))
					actions.push_back(createAttackAction(unit->getUnitId(), adjacent[adj_hex]));
			}
		}

		// get reachable hexes;
		auto& reachable_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getReachableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd,
			current_state->field, false, false, false);
		//reachable_hexes.erase(std::remove(std::begin(reachable_hexes), std::end(reachable_hexes), active_stack.getHex()), std::end(reachable_hexes));

		//auto& field = current_state->field;
		//auto reachable_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getReachableHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd, field, false, false);

		for (auto hex : reachable_hexes) {
			//if (hex == active_stack.getHex())
			//	continue;
			actions.push_back(createWalkAction(hex));
		}

		// get castable spells;
		if (active_stack.canCast())
			throw std::exception("Not implemented yet");

		if (active_stack.canHeroCast())
			throw std::exception("Not implemented yet");

		if (active_stack.canDefend())
			actions.push_back(createDefendAction());

		if (active_stack.canWait())
			actions.push_back(createWaitAction());

		return actions;
	}


	const std::vector<CombatAction> CombatManager::generateActionsForAI() {
		auto& active_stack = getActiveStack();
		const_cast<CombatPathfinder&>(ai->getPathfinder()).pathMap(active_stack.getHex(), current_state->field, false, 254);

		if (!active_stack.canMakeAction())
			return std::vector<CombatAction>();

		ai->calculateFightValueAdvantageOnHexes(active_stack, current_state->attacker, current_state->field);

		actions.clear();
		hexes_to_attack.clear();

		for (auto unit : current_state->attacker.getUnitsPtrs()) {
			const uint8_t hex = ai->chooseHexToMoveForAttack(active_stack, *unit);
			hexes_to_attack.push_back(hex);
		}

		auto& units_to_attack = ai->chooseUnitToAttack(active_stack, current_state->attacker, hexes_to_attack);

		for (auto unit : units_to_attack) {
			auto uid = unit->getUnitId();
			int hex = hexes_to_attack[uid];

			int distance_to_hex = const_cast<CombatPathfinder&>(ai->getPathfinder()).realDistanceBetweenHexes(active_stack.getHex(), hex, current_state->field);
			const bool unit_hex = (active_stack.getHex() == hex);
			int turns = static_cast<int>(unit_hex ? 1 : std::ceil((float)distance_to_hex / active_stack.getCombatStats().spd));

			if (turns == 1)
				actions.push_back(createAttackAction(uid, hex));
			else {
				int walk_distance = ai->chooseWalkDistanceFromPath(active_stack, hex, current_state->field, uid);

				if (walk_distance < active_stack.getCombatStats().spd && active_stack.canWait())
					actions.push_back(createWaitAction());
				else
					actions.push_back(createWalkAction(hex, walk_distance));
			}
		}

		// TODO: make tests to find out the behaviour
		if (actions.empty()) {
			if (active_stack.canWait())
				actions.push_back(createWaitAction());
			else
				actions.push_back(createDefendAction());
		}

		// TODO: for now return only 1 action for ai
		return std::vector<CombatAction>(std::begin(actions), std::begin(actions) + 1);
	}

}; // HotaMechanics