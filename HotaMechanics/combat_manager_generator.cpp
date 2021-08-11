#include "combat_manager.h"


#include "combat_unit.h"
#include "combat_ai.h"
#include "combat_pathfinder.h"

namespace HotaMechanics {
	using namespace Constants;

	const std::vector<CombatAction> CombatManager::generateActionsForPlayer() {
		auto& active_stack = getActiveStack();

		if (!active_stack.canMakeAction())
			return std::vector<CombatAction>();

		std::vector<CombatAction> actions;
		actions.reserve(128);

		if (active_stack.canDefend())
			actions.push_back(createDefendAction());

		if (active_stack.canWait())
			actions.push_back(createWaitAction());

		// get reachable hexes;
		auto reachable_hexes = ai->getReachableHexesForUnit(active_stack);
		//reachable_hexes.erase(std::remove(std::begin(reachable_hexes), std::end(reachable_hexes), active_stack.getHex()), std::end(reachable_hexes));

		//auto& field = current_state->field;
		//auto reachable_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getReachableHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd, field, false, false);
		
		for (auto hex : reachable_hexes) {
			actions.push_back(createWalkAction(hex));
		}

		// get attackable enemy units; 
		// if can shoot then only get all enemy units
		//auto range_hexes = const_cast<CombatPathfinder&>(ai->getPathfinder()).getHexesInRange(activeStack.getHex(), activeStack.getCombatStats().spd + 1);
		auto units_in_range = ai->getEnemyUnitsInAttackRange(active_stack);//getUnitsInRange(CombatSide::DEFENDER, range_hexes);

		for (auto unit : units_in_range) {
			auto adjacent = ai->getPathfinder().getAdjacentHexes(unit->getHex());

			for (auto hex : adjacent) {
				if (hex == active_stack.getHex() || ai->canUnitReachHex(active_stack, hex))
					actions.push_back(createAttackAction(unit->getUnitId(), hex));
			}
		}

		// get castable spells;
		if (active_stack.canCast())
			throw std::exception("Not implemented yet");

		if (active_stack.canHeroCast())
			throw std::exception("Not implemented yet");

		return actions;
	}


	const std::vector<CombatAction> CombatManager::generateActionsForAI() {
		auto& active_stack = getActiveStack();

		if (!active_stack.canMakeAction())
			return std::vector<CombatAction>();

		ai->calculateFightValueAdvantageOnHexes(active_stack, current_state->attacker, current_state->field);

		std::vector<CombatAction> actions;
		actions.reserve(8);
		std::vector<int16_t> hexes_to_attack;
		hexes_to_attack.reserve(16);

		for (auto unit : current_state->attacker.getUnitsPtrs()) {
			const int hex = ai->chooseHexToMoveForAttack(active_stack, *unit);
			hexes_to_attack.push_back(hex);
		}

		auto units_to_attack = ai->chooseUnitToAttack(active_stack, current_state->attacker, hexes_to_attack);

		for (auto unit_id : units_to_attack) {
			int hex = hexes_to_attack[unit_id];

			int distance_to_hex = const_cast<CombatPathfinder&>(ai->getPathfinder()).realDistanceBetweenHexes(active_stack.getHex(), hex, current_state->field);
			const bool unit_hex = (active_stack.getHex() == hex);
			int turns = static_cast<int>(unit_hex ? 1 : std::ceil((float)distance_to_hex / active_stack.getCombatStats().spd));

			if (turns == 1)
				actions.push_back(createAttackAction(unit_id, hex));
			else {
				int walk_distance = ai->chooseWalkDistanceFromPath(active_stack, hex, current_state->field, unit_id);

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

		return actions;
	}

}; // HotaMechanics