#pragma once

#include "combat_field.h"
#include "combat_hero.h"

#include "structures.h"

struct CombatState {
	CombatState(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field)
		: attacker(_attacker), defender(_defender), field(_field) {}

	CombatHero attacker;
	CombatHero defender;
	CombatField field;
	int8 turn{ -1 };
	int8 currentUnit{ -1 };
	int8 unitOrder[84]{ 0 };
	CombatResult result{ CombatResult::NOT_STARTED };
};