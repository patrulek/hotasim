#include "combat_hero.h"

#include <algorithm>
#include <iterator>

void CombatHero::initialize() {
	units.reserve(21);
	stats = hero_template.stats;

	if( army_permutation.permutations.empty() )
		army_permutation = hero_template.generateBaseArmyPermutation();

	// if have artifacts modify stats
	// if have secondary skills modify stats
}

void CombatHero::addUnitFromArmy(UnitPermutation _unit_perm) {
	const auto& unit_template = hero_template.army[_unit_perm.unit_id].unit;
	units.emplace_back(CombatUnit{ unit_template, _unit_perm.unit_number, *this });
	units.back().initUnit();
}

void CombatHero::generateUnitsFromArmy() {
	for (auto unit_perm : army_permutation.permutations)
		addUnitFromArmy(unit_perm);
}

std::vector<const CombatUnit*> CombatHero::getUnits() const {
	std::vector<const CombatUnit*> units_{};
	std::transform(std::begin(units), std::end(units), std::back_inserter(units_), [](const auto& _obj) { return &_obj; });

	return units_;
}

bool CombatHero::isAlive() const {
	return std::any_of(std::begin(units), std::end(units), [](const auto& _unit) { return _unit.isAlive(); });
}

int CombatHero::aliveStacks(CombatHero& hero) const {
	return 1; // check all alive stacks in army
}
