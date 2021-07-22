#pragma once


#include "structures.h"


CombatAction* generateCombatActionsForPlayer(CombatState& state) {
	CombatAction* actions = new CombatAction[256];
	return actions;
}

CombatAction generateCombatActionForAI(CombatState& state) {
	//CombatUnit& unit = getUnit(state);
}