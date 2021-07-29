#pragma once

#include "combat_field.h"
#include "combat_hero.h"

#include "structures.h"

struct CombatState {
	CombatState(const CombatHero& attacker, const CombatHero& defender, const CombatField& field) {
		heroes[0] = attacker;
		heroes[1] = defender;
		this->field = field;
	}

	CombatHero heroes[2];
	CombatField field;
	int8 turn{ -1 };
	int8 currentUnit{ -1 };
	int8 unitOrder[84]{ 0 };
	CombatResult result{ CombatResult::NOT_STARTED };
};