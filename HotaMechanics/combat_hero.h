#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>

namespace HotaMechanics {
	class CombatHero {
	public:
		CombatHero() = delete;
		explicit CombatHero(const Hero& _hero_template, const Constants::CombatSide _side);
		explicit CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const Constants::CombatSide _side);

		CombatHero(const CombatHero& _obj);
		CombatHero(CombatHero&& _obj) noexcept;

		CombatHero& operator=(const CombatHero& _obj); // TODO: remove, needed for tests
		CombatHero& operator=(CombatHero&& _obj); // TODO: remove

		~CombatHero();

		// complex getters ---------------
		const std::vector<const CombatUnit*>& getUnitsPtrs() const;
		const int16_t getUnitId(const CombatUnit& _unit) const;
		const int16_t getGlobalUnitId(const CombatUnit& _unit) const;
		int64_t rehash();
		// -------------------------------

		// check hero state --------------
		const bool isAlive() const;
		const bool canCast() const { return false; }// todo}

		// simple getters -----------------
		const int getMana() const { return stats.primary_stats.mana; }
		const BaseStats& getBaseStats() const { return stats.base_stats; }
		const HeroStats& getStats() const { return stats; }
		const Hero& getTemplate() const { return hero_template; }
		const Constants::CombatSide getCombatSide() const { return side; }
		const ArmyPermutation& getArmyPermutation() const { return army_permutation; }
		// --------------------------------
	private:
		void initialize();
		void addUnitFromArmy(UnitPermutation _unit_perm);
		ArmyPermutation generateBaseArmyPermutation() const;
		void generateUnitsFromArmy();

		const Hero& hero_template;
		const ArmyPermutation& army_permutation;

		HeroStats stats;
		Constants::CombatSide side;

		std::vector<CombatUnit> units;
		std::vector<const CombatUnit*> unit_ptrs;
	};
};