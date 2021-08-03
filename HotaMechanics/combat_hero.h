#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>

class CombatHero {
private:

	Hero hero_template;
	PrimaryStats stats;
	std::vector<CombatUnit> units;
	std::array<CombatUnit*, 21> units_placement;
	CombatSide side;
	ArmyPermutation army_permutation;

	void initialize();
	void addUnitFromArmy(UnitPermutation _unit_perm);
public:

	CombatHero() = default;
	CombatHero(const Hero& hero_template)
			: hero_template(hero_template) {
		initialize();
		generateUnitsFromArmy();
	}

	CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation)
		: hero_template(_hero_template), army_permutation(_army_permutation) {
		initialize();
		generateUnitsFromArmy();
	}

	std::vector<const CombatUnit*> getUnits() const;
	const PrimaryStats& getStats() const {
		return stats;
	}

	CombatSide getCombatSide() const {
		return side;
	}

	void generateUnitsFromArmy();

	int getUnitId(const CombatUnit& unit) const {
		auto it = std::find_if(std::begin(units_placement), std::end(units_placement), [&unit](auto _obj) { return _obj == &unit; });
		bool found = (it != std::end(units_placement));
		return (it - std::begin(units_placement)) * found - !found;
	}

	bool isAlive() const;

	bool canCast() const {
		return false; // todo
	}

	int aliveStacks(CombatHero& hero) const;
};