#pragma once

#include "combat_field.h"
#include "combat_hero.h"

#include "structures.h"

#include <list>

struct CombatState {
	CombatState(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field)
		: attacker(_attacker), defender(_defender), field(_field) {}

	CombatState() = delete;
	CombatState(const CombatState& _obj) = default;
	CombatState(CombatState&& _obj) = default;

	CombatState& operator=(const CombatState& _obj) = default;
	CombatState& operator=(CombatState&& _obj) = default;

	CombatHero attacker;
	CombatHero defender;
	CombatField field;
	int turn{ -1 };
	std::list<int> order;	// for attacker it will be 0-20; for defender it will be 21-41
	CombatResult result{ CombatResult::NOT_STARTED };
};