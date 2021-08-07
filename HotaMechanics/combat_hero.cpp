#include "combat_hero.h"

#include <algorithm>
#include <iterator>

namespace HotaMechanics {
	using namespace Constants;

	CombatHero::CombatHero(const Hero& hero_template, const CombatSide _side)
		: hero_template(hero_template), side(_side) {
		initialize();
		generateUnitsFromArmy();
	}

	CombatHero::CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const CombatSide _side)
		: hero_template(_hero_template), army_permutation(_army_permutation), side(_side) {
		initialize();
		generateUnitsFromArmy();
	}

	CombatHero::CombatHero(const CombatHero& _obj)
		: hero_template(_obj.hero_template) {
		stats = _obj.stats;

		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));

		side = _obj.side;
		army_permutation = _obj.army_permutation;
	}

	// TODO: fix it
	CombatHero& CombatHero::operator=(const CombatHero& _obj) {
		if (this == &_obj)
			return *this;

		hero_template = _obj.hero_template;
		stats = _obj.stats;

		units.clear();
		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));

		side = _obj.side;
		army_permutation = _obj.army_permutation;
		return *this;
	}

	CombatHero::CombatHero(CombatHero&& _obj) noexcept
		: hero_template(_obj.hero_template) {
		stats = std::move(_obj.stats);

		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));
		_obj.units.clear();

		side = std::move(_obj.side);
		army_permutation = std::move(_obj.army_permutation);
	}

	// TODO: fix it
	CombatHero& CombatHero::operator=(CombatHero&& _obj) {
		if (this == &_obj)
			return *this;

		hero_template = std::move(_obj.hero_template);
		stats = std::move(_obj.stats);

		units.clear();
		for (auto unit : _obj.units)
			units.emplace_back(CombatUnit(unit, *this));
		_obj.units.clear();

		side = std::move(_obj.side);
		army_permutation = std::move(_obj.army_permutation);
		return *this;
	}

	void CombatHero::initialize() {
		units.reserve(21);
		unit_ptrs.reserve(21);
		stats = hero_template.stats;

		if (army_permutation.permutations.empty())
			army_permutation = generateBaseArmyPermutation();

		// if have artifacts modify stats
		// if have secondary skills modify stats
	}

	ArmyPermutation CombatHero::generateBaseArmyPermutation() const {
		ArmyPermutation permutation;

		for (int8_t i = 0; i < hero_template.army.size(); ++i)
			permutation.permutations.push_back(UnitPermutation{ i, i, hero_template.army[i].stack_number });

		return permutation;
	}

	void CombatHero::generateUnitsFromArmy() {
		for (auto unit_perm : army_permutation.permutations)
			addUnitFromArmy(unit_perm);
	}

	void CombatHero::addUnitFromArmy(UnitPermutation _unit_perm) {
		const auto& unit_template = hero_template.army[_unit_perm.unit_id].unit;
		units.emplace_back(CombatUnit{ unit_template, _unit_perm.unit_number, *this });
		units.back().initUnit();
	}


	const std::vector<const CombatUnit*> CombatHero::getUnits() const {
		std::vector<const CombatUnit*> units_{};
		std::transform(std::begin(units), std::end(units), std::back_inserter(units_), [](const auto& _obj) { return &_obj; });

		return units_;
	}

	const std::vector<const CombatUnit*> CombatHero::getUnitsPtrs() {
		unit_ptrs.clear();
		std::transform(std::begin(units), std::end(units), std::back_inserter(unit_ptrs), [](const auto& _obj) { return &_obj; });

		return unit_ptrs;
	}

	const int CombatHero::getUnitId(const CombatUnit& _unit) const {
		auto it = std::find_if(std::begin(units), std::end(units), [&_unit](const CombatUnit& _obj) { return &_obj == &_unit; });
		bool found = (it != std::end(units));
		return (it - std::begin(units)) * found - !found;
	}

	const int CombatHero::getGlobalUnitId(const CombatUnit& _unit) const {
		auto id = getUnitId(_unit);
		return id + (21 * (side == CombatSide::DEFENDER));
	}

	const bool CombatHero::isAlive() const {
		return std::any_of(std::begin(units), std::end(units), [](const auto& _unit) { return _unit.isAlive(); });
	}
}; // HotaMechanics