#pragma once

#include "combat_field.h"
#include "combat_hero.h"

#include "structures.h"

struct CombatState {
	CombatHero heroes[2];
	CombatField field;
	int8 turn{ -1 };
	int8 currentUnit{ -1 };
	int8 unitOrder[84]{ 0 };
	CombatResult result{ CombatResult::NOT_STARTED };
};