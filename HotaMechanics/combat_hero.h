#pragma once

#include "structures.h"
#include "combat_unit.h"


struct CombatHero {
	PrimaryStats stats{};
	SecondarySkills skills{};
	SpellBook spells{};
	Equipment artifacts{};
	CombatUnit units[21]{};

	CombatHero() = default;



	bool isAlive(CombatHero& hero) {
		return true; // check if has any unit alive
	}

	int aliveStacks(CombatHero& hero) {
		return 1; // check all alive stacks in army
	}
};