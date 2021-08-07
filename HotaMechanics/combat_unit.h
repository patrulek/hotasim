#pragma once


#include "structures.h"

#include <string>
#include <array>
#include <vector>

namespace HotaMechanics {
	class CombatHex;
	class CombatHero;
	class CombatField;

	struct CombatUnitState {
		bool is_alive{ false };
		bool is_summon{ false };
		bool is_clone{ false };
		bool morale{ false };
		bool waiting{ false };
		bool done{ false };
		bool defending{ false };
		bool sacrificed{ false };
		bool retaliated{ false };
		bool applied_hero_stats{ false };
	};

	class CombatUnit {
	public:
		CombatUnit() = delete;
		explicit CombatUnit(const Unit& _unit_template, const int _stack_number, const CombatHero& _hero);
		explicit CombatUnit(const CombatUnit& _unit, const CombatHero& _hero);
		CombatUnit(CombatUnit&& _unit, const CombatHero& _hero);
		
		// state change by action --------
		void moveTo(const int _target_hex);

		void defend();
		void wait();

		void applyDamage(const int _damage);
		// -------------------------------

		// state change ------------------
		void resetState();
		void initUnit();
		void applyHeroStats();
		void setDone();
		void unsetDone();
		void setRetaliated();
		// -------------------------------

		// check unit state --------------
		const bool canDefend() const { return !state.defending; }
		const bool isAlive() const { return state.is_alive; }
		const bool canMakeAction() const { return state.is_alive && !state.done; }
		const bool canWait() const {	return !state.waiting; }
			// C8DE0 - get unit number shots and check if arrow tower
		const bool canShoot() const { return stats.combat_stats.shots > 0; }// 0 && false; // unit->getNumberShots && unit->isArrowTower()}
		const bool canFly() const { return false; }// flags & 2; }
		const bool canCast() const { return false; } // todo}
		const bool canRetaliate() const { return !state.retaliated; }
		// -------------------------------

		// check hero state --------------
		bool canHeroCast() const;
		// -------------------------------

		// check unit abilities ----------
		const bool isDoubleWide() const { return false; } // TODO }
		const bool isShooter() const { return false; } // TODO}

			// C86D0 - check if arrow tower, ballista or 150 (cannon? undefined?)
		const bool isShootingSiege() const { return false; } // isBallista, isArrowTower, isCannon(150 id)? }
		const bool isArrowTower() const { return false; } // creaturetype == 149}
		const bool isFlyer() const { return false; } // TODO }
		// -------------------------------

		// complex getters ---------------
			// 42270 - not sure
		const int getAttackGain() const;
			// 42380 - additionaly checking if we have moat;
		const int getDefenseGain() const;
		const float getBaseAverageDmg() const;
		const float getFightValuePerOneHp() const;
		const float getFightValuePerUnitStack() const;
		const int getUnitStackHP() const;
		// -------------------------------

		//	hero getters ------------------
		const int getUnitId() const;
		const int getGlobalUnitId() const;
		const Constants::CombatSide getCombatSide() const;
		const Constants::CombatSide getEnemyCombatSide() const;
		// -------------------------------

		// simple getters ----------------
		const CombatStats& getCombatStats() const { return stats.combat_stats; }
		const BaseStats& getBaseStats() const { return stats.base_stats; }
		const PrimaryStats& getPrimaryStats() const { return stats.primary_stats; }
		int16_t getHealthLost() const { return health_lost; }
		int16_t getHex() const { return hex; };
		const Unit& getTemplate() const { return unit_template; }
		int16_t getStackNumber() const { return stack_number; }
		float getFightValue() const { return unit_template.stats.fight_value; }
		const CombatHero* getHero() const { return hero; }
		// -------------------------------

		// util --------------------------
		std::string toString() const;
		// -------------------------------
	private:
		Unit unit_template;
		const CombatHero* hero;

		UnitStats stats;
		int16_t hex{ -1 };
		int16_t stack_number{ 0 };
		int16_t health_lost{ 0 };
		CombatUnitState state{ 0 };
		std::array<Constants::Spells, 8> active_spells{};
	};
} // HotaMechanics