#include "combat_hero.h"

#include <algorithm>
#include <iterator>
#include <iostream>

namespace HotaMechanics {
	using namespace Constants;

	CombatHero::CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const CombatSide _side)
		: hero_template(&_hero_template), army_permutation(&_army_permutation), side(_side) {
		initialize();
		generateUnitsFromArmy();
	}

	CombatHero::CombatHero(const CombatHero& _obj)
		: hero_template(_obj.hero_template), army_permutation(_obj.army_permutation) {

		initialize();
		stats = _obj.stats;
		side = _obj.side;

		units.clear();
		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(*unit, *this);
			unit_ptrs.push_back(&(units.back()));
		}
	}

	CombatHero::CombatHero(CombatHero&& _obj) noexcept
		: hero_template(std::move(_obj.hero_template)), army_permutation(std::move(_obj.army_permutation)) {
		initialize();
		stats = std::move(_obj.stats);
		side = std::move(_obj.side);

		units.clear();
		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(*unit, *this);
			unit_ptrs.push_back(&(units.back()));
		}
		_obj.units.clear();
		_obj.unit_ptrs.clear();
	}

	void CombatHero::initialize() {
		units.reserve(BASE_ARMY_SIZE);
		unit_ptrs.reserve(BASE_ARMY_SIZE);
		stats = hero_template->stats;

		// if have artifacts modify stats
		// if have secondary skills modify stats
	}

	void CombatHero::generateUnitsFromArmy() {
		for (auto& unit_perm : army_permutation->permutations)
			addUnitFromArmy(unit_perm);
	}

	void CombatHero::addUnitFromArmy(const UnitPermutation& _unit_perm) {
		const auto unit_template = hero_template->army[_unit_perm.unit_id].unit;
		units.emplace_back(*unit_template, *this, _unit_perm.unit_number);
		units.back().initUnit();
		unit_ptrs.push_back(&(units.back()));
	}

	const std::vector<const CombatUnit*>& CombatHero::getUnitsPtrs() const {
		return unit_ptrs;
	}

	const UnitId CombatHero::getUnitId(const CombatUnit& _unit) const {
		auto it = std::find_if(std::begin(units), std::end(units), [&_unit](const CombatUnit& _obj) { return &_obj == &_unit; });
		if (it != std::end(units))
			return static_cast<UnitId>(it - std::begin(units));
		return INVALID_UNIT_ID;
	}

	const UnitId CombatHero::getGlobalUnitId(const CombatUnit& _unit) const {
		if (side == CombatSide::ATTACKER) 
			return getUnitId(_unit);
		return getUnitId(_unit) + GUID_OFFSET;
	}

	const bool CombatHero::isAlive() const {
		return std::any_of(std::begin(units), std::end(units), [](const auto& _unit) { return _unit.isAlive(); });
	}

	Hash CombatHero::rehash() {
		Hash hash = std::hash<Hash>{}(stats.primary_stats.mana);
		hash ^= std::hash<Hash>{}(stats.base_stats.stats);

		for (auto& unit : units)
			hash ^= unit.rehash();

		return hash;
	}
}; // HotaMechanics