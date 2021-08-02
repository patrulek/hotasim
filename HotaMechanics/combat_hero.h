#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>

enum class CombatSide {
	ATTACKER, DEFENDER
};

class CombatHero {
public:
	Hero hero_template;
	PrimaryStats stats{};
	CombatUnit units[21];
	CombatSide side;

	CombatHero() = default;
	CombatHero(const Hero& hero_template)
		: hero_template(hero_template) {}

	std::vector<const CombatUnit*> getActiveUnits() const;

	CombatSide getCombatSide() const {
		return side;
	}

	int getUnitId(const CombatUnit& unit) const {
		for (int i = 0; i < 21; ++i) {
			if (&units[i] == &unit) // todo
				return i;
		}

		return -1;
	}

	bool isAlive(CombatHero& hero) const;

	bool canCast() const {
		return false; // todo
	}

	int aliveStacks(CombatHero& hero) const;
};