#pragma once


#include "structures.h"

#include <vector>

enum class CombatActionType {
	WALK, ATTACK, WAIT, DEFENSE, SPELLCAST
};

class CombatAction {
public:
	CombatActionType action;
	int param1;
	int target;
	int param2;

};