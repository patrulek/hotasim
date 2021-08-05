#include "combat_manager.h"

#include "combat_unit.h"

namespace HotaMechanics {
	using namespace Constants;

	void CombatManager::processUnitAction(const CombatAction& _action) {
		if (_action.action == CombatActionType::DEFENSE)
			processDefendAction(_action);
		else if (_action.action == CombatActionType::WAIT)
			processWaitAction(_action);
		else if (_action.action == CombatActionType::WALK)
			processWalkAction(_action);
		else if (_action.action == CombatActionType::SPELLCAST)
			processSpellCastAction(_action);
		else if (_action.action == CombatActionType::ATTACK)
			processAttackAction(_action);
	}

	void CombatManager::processCombatAction(const CombatAction& _action) {
		if (_action.action == CombatActionType::PRE_BATTLE)
			processPreBattleAction(_action);
		else if (_action.action == CombatActionType::PRE_TURN)
			processPreTurnAction(_action);
	}

	void CombatManager::processPreBattleAction(const CombatAction& _action) {
		// TODO:
			// apply secondary skills for units from hero
			// apply precombat artifacts spells
		current_state->turn = 0;
		orderUnitsInTurn();
		//std::cout << "Processed action: PRE_BATTLE\n\n-------------------\n\n";
	}

	void CombatManager::processPreTurnAction(const CombatAction& _action) {
		for (auto& unit : current_state->attacker.getUnits())
			const_cast<CombatUnit*>(unit)->resetState();

		for (auto& unit : current_state->defender.getUnits())
			const_cast<CombatUnit*>(unit)->resetState();

		// reactivate spellbook
			// decrease spell active on units
		++current_state->turn;
		orderUnitsInTurn();
		//std::cout << "Processed action: PRE_TURN (" << current_state->turn << ")\n\n-----------------\n\n";
	}

	void CombatManager::processWaitAction(const CombatAction& _action) {
		makeUnitWait();

		auto& active_stack = getActiveStack();
		//std::cout << "Processed action: WAIT (" << active_stack.to_string().c_str() << ")\n\n";
	}

	void CombatManager::processWalkAction(const CombatAction& _action) {
		auto& active_stack = getActiveStack();
		if (active_stack.canFly())
			makeUnitFly(_action.target);
		else
			makeUnitWalk(_action.target, _action.param1);
		//std::cout << "Processed action: WALK (" << active_stack.to_string().c_str() << ") to pos: " << _action.target << "\n\n";
	}

	void CombatManager::processDefendAction(const CombatAction& _action) {
		makeUnitDefend();

		auto& active_stack = getActiveStack();
		//std::cout << "Processed action: DEFENSE (" << active_stack.to_string().c_str() << ")\n\n";
	}

	void CombatManager::processSpellCastAction(const CombatAction& _action) {
		//std::cout << "Processed action: SPELLCAST (need to be implemented)\n";
	}

	void CombatManager::processAttackAction(const CombatAction& _action) {
		makeUnitAttack(_action.param1, _action.target);

		auto& active_stack = getActiveStack();
		//std::cout << "Processed action: ATTACK (" << active_stack.to_string().c_str() << ") on unit: " 
		//			 << _action.param1 << " at pos: " << _action.target << "\n\n";
	}
}; // HotaMechanics