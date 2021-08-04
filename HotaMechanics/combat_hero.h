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
	CombatSide side;
	ArmyPermutation army_permutation;

	void initialize();
	void addUnitFromArmy(UnitPermutation _unit_perm);
public:

	CombatHero() = delete;
	explicit CombatHero(const Hero& hero_template, const CombatSide _side)
			: hero_template(hero_template), side(_side) {
		initialize();
		generateUnitsFromArmy();
	}

	explicit CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const CombatSide _side)
		: hero_template(_hero_template), army_permutation(_army_permutation), side(_side) {
		initialize();
		generateUnitsFromArmy();
	}

	CombatHero(const CombatHero& _obj) {
		hero_template = _obj.hero_template;
		stats = _obj.stats;

		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));

		side = _obj.side;
		army_permutation = _obj.army_permutation;
	}

	CombatHero(CombatHero&& _obj) noexcept {
		hero_template = std::move(_obj.hero_template);
		stats = std::move(_obj.stats);

		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));
		_obj.units.clear();

		side = std::move(_obj.side);
		army_permutation = std::move(_obj.army_permutation);
	}

	CombatHero& operator=(const CombatHero& _obj) = default;
	CombatHero& operator=(CombatHero&& _obj) = default;

	std::vector<const CombatUnit*> getUnits() const;
	const PrimaryStats& getStats() const {
		return stats;
	}

	CombatSide getCombatSide() const {
		return side;
	}

	void generateUnitsFromArmy();

	int getUnitId(const CombatUnit& unit) const {
		auto ptr_units = getUnits();
		auto it = std::find_if(std::begin(ptr_units), std::end(ptr_units), [&unit](auto _obj) { return _obj == &unit; });
		bool found = (it != std::end(ptr_units));
		return (it - std::begin(ptr_units)) * found - !found;
	}

	bool isAlive() const;

	bool canCast() const {
		return false; // todo
	}

	int getMana() const {
		return stats.mana;
	}

	const Hero& getTemplate() const {
		return hero_template;
	}

	int aliveStacks(CombatHero& hero) const;
};