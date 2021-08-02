#include "combat_hero.h"


std::vector<const CombatUnit*> CombatHero::getActiveUnits() const {
	auto units_ = std::vector<const CombatUnit*>();

	for (int i = 0; i < 21; ++i) {
		if (units[i].applied_hero_stats) // todo
			units_.emplace_back(&units[i]);
	}

	return units_;
}

bool CombatHero::isAlive(CombatHero& hero) const {
	return true; // check if has any unit alive
}

int CombatHero::aliveStacks(CombatHero& hero) const {
	return 1; // check all alive stacks in army
}
