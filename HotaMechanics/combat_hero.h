#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>

struct HeroArmy {
	std::array<Unit, 7> units;
	std::array<int, 7> number;
};

struct Hero {
	PrimaryStats stats{};
	HeroSkills skills{};
	SpellBook spells{};
	Equipment artifacts{};
	HeroArmy army;
};

class CombatHero {
public:
	Hero hero_template;
	PrimaryStats stats{};
	CombatUnit units[21];

	CombatHero() = default;
	CombatHero(const Hero& hero_template)
		: hero_template(hero_template) {}

	std::vector<CombatUnit> getActiveUnits() const;

	bool isAlive(CombatHero& hero) const;

	int aliveStacks(CombatHero& hero) const;
};