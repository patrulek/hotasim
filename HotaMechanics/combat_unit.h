#pragma once


#include "structures.h"

#include <string>
#include <array>
#include <vector>
#include <memory>

namespace HotaMechanics {
	class CombatHex;
	class CombatHero;
	class CombatField;

	union CombatUnitState {
		int16_t state;
		struct {
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
		} flags;

		int16_t flagsToValue() {
			state = (1 << 0) * flags.is_alive | (1 << 1) * flags.is_summon | (1 << 2) * flags.is_clone | (1 << 3) * flags.morale
				| (1 << 4) * flags.waiting | (1 << 5) * flags.done | (1 << 6) * flags.defending | (1 << 7) * flags.sacrificed
				| (1 << 8) * flags.retaliated | (1 << 9) * flags.applied_hero_stats;
			return state;
		}
	};

	class CombatUnit {
	public:
		CombatUnit() = delete;
		explicit CombatUnit(const Unit& _unit_template, const CombatHero& _hero, const int16_t _stack_number = 0);
		CombatUnit(const CombatUnit& _unit, const CombatHero& _hero);
		CombatUnit(CombatUnit&& _unit, const CombatHero& _hero);
		
		// state change by action --------
		void moveTo(const HexId _target_hex);

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
		const bool canDefend() const { return !state.flags.defending; }
		const bool isAlive() const { return state.flags.is_alive; }
		const bool canMakeAction() const { return state.flags.is_alive && !state.flags.done; }
		const bool canWait() const {	return !state.flags.waiting; }
			// C8DE0 - get unit number shots and check if arrow tower
		const bool canShoot() const { return stats.combat_stats.shots > 0; }// 0 && false; // unit->getNumberShots && unit->isArrowTower()}
		const bool canFly() const { return false; }// flags & 2; }
		const bool canCast() const { return false; } // todo}
		const bool canRetaliate() const { return !state.flags.retaliated; }
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
		const int16_t getAttackGain() const;
			// 42380 - additionaly checking if we have moat;
		const int16_t getDefenseGain() const;
		const float getBaseAverageDmg() const;
		const float getFightValuePerOneHp() const;
		const float getFightValuePerUnitStack() const;
		const int getUnitStackHP() const;

		Hash rehash();
		// -------------------------------

		//	hero getters ------------------
		const HexId getGlobalUnitId() const;
		const Constants::CombatSide getCombatSide() const;
		const Constants::CombatSide getEnemyCombatSide() const;
		// -------------------------------

		// simple setters ----------------
		void setUnitState(const CombatUnitState& _state) { state = _state; }
		void setStats(const UnitStats& _stats) { stats = _stats; }
		void setStackNumber(const int16_t _stack_number) { stack_number = _stack_number; }
		void setHealthLost(const int16_t _health_lost) { health_lost = _health_lost; }
		void setHex(const HexId _hex) { hex = _hex; }
		// -------------------------------

		// simple getters ----------------
		const CombatStats& getCombatStats() const { return stats.combat_stats; }
		const BaseStats& getBaseStats() const { return stats.base_stats; }
		const PrimaryStats& getPrimaryStats() const { return stats.primary_stats; }
		const UnitStats& getStats() const { return stats; }
		const CombatUnitState getState() const { return state; }
		int16_t getHealthLost() const { return health_lost; }
		HexId getHex() const { return hex; };
		const Unit& getTemplate() const { return *unit_template; }
		int16_t getStackNumber() const { return stack_number; }
		float getFightValue() const { return unit_template->stats.fight_value; }
		const CombatHero& getHero() const { return *hero; }
		const UnitId getUnitId() const { return uid; }
		// -------------------------------

		// util --------------------------
		std::string toString() const;
		// -------------------------------
	private:
		UnitId uid{ Constants::INVALID_UNIT_ID };
		const Unit* const unit_template{ nullptr };
		const CombatHero* hero{ nullptr };

		UnitStats stats{ 0 };
		HexId hex{ Constants::INVALID_HEX_ID };
		int16_t stack_number{ 0 };
		int16_t health_lost{ 0 };
		CombatUnitState state{ 0 };
		std::array<Constants::Spells, 8> active_spells{};
	};
} // HotaMechanics