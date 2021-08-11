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
		army_permutation = _obj.army_permutation;

		initialize();

		stats = _obj.stats;
		side = _obj.side;

		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(CombatUnit(*unit, *this));
			unit_ptrs.push_back(&(units.back()));
		}
	}

	// TODO: fix it
	CombatHero& CombatHero::operator=(const CombatHero& _obj) {
		if (this == &_obj)
			return *this;

		//initialize();
		hero_template = _obj.hero_template;
		stats = _obj.stats;
		side = _obj.side;
		army_permutation = _obj.army_permutation;

		//units.clear();
		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(CombatUnit(*unit, *this));
			unit_ptrs.push_back(&(units.back()));
		}
		return *this;
	}

	CombatHero::CombatHero(CombatHero&& _obj) noexcept
		: hero_template(_obj.hero_template) {
		//initialize();
		stats = std::move(_obj.stats);
		side = std::move(_obj.side);
		army_permutation = std::move(_obj.army_permutation);

		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(CombatUnit(*unit, *this));
			unit_ptrs.push_back(&(units.back()));
		}
		_obj.units.clear();
	}

	// TODO: fix it
	CombatHero& CombatHero::operator=(CombatHero&& _obj) {
		if (this == &_obj)
			return *this;

		//initialize();
		hero_template = std::move(_obj.hero_template);
		stats = std::move(_obj.stats);
		side = std::move(_obj.side);
		army_permutation = std::move(_obj.army_permutation);

		//units.clear();
		unit_ptrs.clear();
		for (auto unit : _obj.getUnitsPtrs()) {
			units.emplace_back(CombatUnit(*unit, *this));
			unit_ptrs.push_back(&(units.back()));
		}
		_obj.units.clear();
		return *this;
	}

	void CombatHero::initialize() {
		units.reserve(7);
		unit_ptrs.reserve(7);
		stats = hero_template.stats;

		if (army_permutation.permutations.empty())
			army_permutation = generateBaseArmyPermutation();

		// if have artifacts modify stats
		// if have secondary skills modify stats
	}

	ArmyPermutation CombatHero::generateBaseArmyPermutation() const {
		ArmyPermutation permutation;

		for (int8_t i = 0; i < hero_template.army.size(); ++i)
			permutation.permutations.emplace_back(i, i, hero_template.army[i].stack_number);

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
		unit_ptrs.push_back(&(units.back()));
	}

	const std::vector<const CombatUnit*>& CombatHero::getUnitsPtrs() const {
		return unit_ptrs;
	}

	const int16_t CombatHero::getUnitId(const CombatUnit& _unit) const {
		auto it = std::find_if(std::begin(units), std::end(units), [&_unit](const CombatUnit& _obj) { return &_obj == &_unit; });
		bool found = (it != std::end(units));
		return (int16_t)((it - std::begin(units)) * found - !found);
	}

	const int16_t CombatHero::getGlobalUnitId(const CombatUnit& _unit) const {
		auto id = getUnitId(_unit);
		return id + (21 * (side == CombatSide::DEFENDER));
	}

	const bool CombatHero::isAlive() const {
		return std::any_of(std::begin(units), std::end(units), [](const auto& _unit) { return _unit.isAlive(); });
	}
}; // HotaMechanics