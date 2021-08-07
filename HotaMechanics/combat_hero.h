#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>

namespace HotaMechanics {
	class CombatHero {
	public:
		CombatHero() = delete;
		explicit CombatHero(const Hero& hero_template, const Constants::CombatSide _side);
		explicit CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const Constants::CombatSide _side);

		CombatHero(const CombatHero& _obj);
		CombatHero(CombatHero&& _obj) noexcept;

		CombatHero& operator=(const CombatHero& _obj);
		CombatHero& operator=(CombatHero&& _obj);


		// complex getters ---------------
		const std::vector<const CombatUnit*> getUnits() const;
		const std::vector<const CombatUnit*> getUnitsPtrs();
		const int getUnitId(const CombatUnit& _unit) const;
		const int getGlobalUnitId(const CombatUnit& _unit) const;
		// -------------------------------

		// check hero state --------------
		const bool isAlive() const;
		const bool canCast() const { return false; }// todo}

		// simple getters -----------------
		const int getMana() const { return stats.primary_stats.mana; }
		const BaseStats& getBaseStats() const { return stats.base_stats; }
		const Hero& getTemplate() const { return hero_template; }
		const Constants::CombatSide getCombatSide() const { return side; }
		// --------------------------------
	private:
		void initialize();
		void addUnitFromArmy(UnitPermutation _unit_perm);
		ArmyPermutation generateBaseArmyPermutation() const;
		void generateUnitsFromArmy();

		Hero hero_template;

		HeroStats stats;
		ArmyPermutation army_permutation;
		Constants::CombatSide side;

		std::vector<CombatUnit> units;
		std::vector<const CombatUnit*> unit_ptrs;
	};
};