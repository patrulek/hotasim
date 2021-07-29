#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>

class CombatHero {
public:
	PrimaryStats stats{};
	HeroSkills skills{};
	SpellBook spells{};
	Equipment artifacts{};
	CombatUnit units[21];

	CombatHero() = default;

	std::vector<CombatUnit> getActiveUnits() const;

	bool isAlive(CombatHero& hero) const;

	int aliveStacks(CombatHero& hero) const;
};