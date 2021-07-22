#pragma once

#include <memory>

#include "structures.h"
#include "combat_ai.h"
#include "combat_hero.h"
#include "combat_field.h"
#include "combat_state.h"

class CombatAI;

class CombatManager {

public:
	CombatManager(const CombatHero& attacker, const CombatHero& defender, const CombatField& field);



	CombatState nextState();

	CombatUnit& getActiveStack() {
		int unitId = current_state->unitOrder[current_state->currentUnit];
		int side = unitId / 21;
		return current_state->heroes[side].units[unitId % 21];
	}


	CombatState duplicateCurrentState() const { return CombatState(*current_state); }
	CombatState& getCurrentState() const { return *current_state; }
	const CombatState& getInitialState() const { return init_state; }





	CombatUnit& nextUnit(CombatState& state) {
		// get next unit in turn
		// 

		return state.heroes[0].units[0]; // 
	}

	bool isUnitMove(CombatState& state) {
		return state.currentUnit != -1; // check if unit move or turnupdate
	}

	bool isPlayerMove(CombatState& state) {
		return isUnitMove(state) && state.unitOrder[state.currentUnit] < 21;
	}

	void resolveAction(CombatState& state, CombatAction action) {

	}

	bool isTacticsState(CombatState& state) const {
		return false;
	}

	// 1EC62
	int getSideStacksNumber(CombatState& state, int side) const {
		return state.heroes[side].aliveStacks(state.heroes[side]);
	}

	CombatState nextStateByAction(CombatState& state, CombatAction action) {
		//CombatState state_ = duplicate(state);
		//resolveAction(state_, action);
		//return state_;
	}

private:
	void setCombatResult();

	void orderUnitsInTurn();
	void preTurnUpdate();
	void updateTurn();

	void updateCombat();


	bool isNewTurn();

	bool isNewCombat();

	void preCombatUpdate() {
		// update combat field
		// place units over combat field
		// apply secondary skills for units from hero
		// apply precombat artifacts spells
	}

	const CombatState& init_state;
	std::unique_ptr<CombatState> current_state;
	std::unique_ptr<CombatAI> ai;
};

/*
FLOW:

CombatState initState = initCombat(...); // before battle

CombatState state = initState;

while( state.result == (NOT_STARTED || IN_PROGRESS) ) {
	if( isUnitMove(state) )
		state = nextStateByAction(nextState, action);
	else
		state = nextState(state);
}


*/