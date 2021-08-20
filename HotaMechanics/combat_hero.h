#pragma once

#include "structures.h"
#include "combat_unit.h"

#include <vector>
#include <array>
#include <memory>

namespace HotaMechanics {
	class CombatHero {
	public:
		CombatHero() = delete; 
		explicit CombatHero(const Hero& _hero_template, const ArmyPermutation& _army_permutation, const Constants::CombatSide _side);

		CombatHero(const CombatHero& _obj);
		CombatHero(CombatHero&& _obj) noexcept;

		~CombatHero() = default;

		// complex getters ---------------
		const std::vector<const CombatUnit*>& getUnitsPtrs() const;
		const UnitId getUnitId(const CombatUnit& _unit) const;
		const UnitId getGlobalUnitId(const CombatUnit& _unit) const;
		Hash rehash();
		// -------------------------------

		// check hero state --------------
		const bool isAlive() const;
		const bool canCast() const { return false; }// todo}

		// simple getters -----------------
		const int16_t getMana() const { return stats.primary_stats.mana; }
		const BaseStats& getBaseStats() const { return stats.base_stats; }
		const HeroStats& getStats() const { return stats; }
		const Hero& getTemplate() const { return *hero_template; }
		const Constants::CombatSide getCombatSide() const { return side; }
		const ArmyPermutation& getArmyPermutation() const { return *army_permutation; }
		// --------------------------------
	private:
		void initialize();
		void addUnitFromArmy(const UnitPermutation& _unit_perm);
		void generateUnitsFromArmy();

		const Hero* const hero_template{ nullptr };
		const ArmyPermutation* const army_permutation{ nullptr };

		HeroStats stats{ 0 };
		Constants::CombatSide side{ Constants::CombatSide::ATTACKER };

		std::vector<CombatUnit> units{};
		std::vector<const CombatUnit*> unit_ptrs{};
	};
};