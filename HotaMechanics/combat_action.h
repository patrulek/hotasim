#pragma once


#include "structures.h"

#include <vector>

enum class CombatActionType {
	WALK, ATTACK, WAIT, DEFENSE, SPELLCAST, PRE_BATTLE, PRE_TURN
};

class CombatAction {
public:
	CombatActionType action;
	int param1; // unit_id (target for attack/spellcast)
	int target; // hex_id (for walk/melee attack/spellcast)
	int param2; // true/false (whether action ends unit turn; hero spellcast dont)
};