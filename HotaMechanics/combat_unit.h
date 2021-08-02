#pragma once


#include "structures.h"

#include <array>
#include <vector>

class CombatHex;
class CombatHero;
class CombatField;
enum class CombatSide;

class CombatUnit {
public:
	Unit unit_template{};
	UnitState state{ 0 };
	PrimaryStats currentStats;
	uint16 stackNumber{ 0 };
	uint16 health_lost{ 0 };
	int hexId{ -1 };
	std::array<SpellID, 8> active_spells{};
	bool applied_hero_stats{ false };

	/*const*/ CombatHero*/*&*/ hero;

	CombatUnit(const Unit& unitTemplate)
		: unit_template(unitTemplate), currentStats(unitTemplate.stats) {};
	CombatUnit(const Unit& unitTemplate, /*const*/ CombatHero& hero)
		: unit_template(unitTemplate), currentStats(unitTemplate.stats), hero(&hero) {};
	CombatUnit(const CombatUnit& obj) = default;
	CombatUnit() = default;

	CombatSide getCombatSide() const;

	int getUnitId() const;

	void applyDamage(int damage);

	float getBaseAverageDmg() const;

	const std::vector<int> getHexesInSpeedRange(const CombatField& field) const;
	const std::vector<int> getHexesInAttackRange(const CombatField& field) const;

	float getSingleUnitFightValue() const {
		return unit_template.fightValue;
	}

	float getSingleUnitFightValuePerOneHp() const {
		return getSingleUnitFightValue() / currentStats.hp;
	}

	float getStackUnitFightValue() const {
		return calculateStackHP() * getSingleUnitFightValuePerOneHp();
	}

	int calculateStackHP() const {
		return stackNumber * currentStats.hp - health_lost;
	}

	void applyHeroStats(const PrimaryStats& hero_stats) {
		currentStats.atk += (hero_stats.atk * !applied_hero_stats);
		currentStats.def += (hero_stats.def * !applied_hero_stats);
		applied_hero_stats = true;
	}

	void initUnit() {
		state.is_alive = true;
	}

	void applySpell(const int spell_id, const int power) {
		return;
	}

	void deactiveSpell(const int spell_id) {
		return;
	}

	bool canCast() const {
		return false; // todo
	}

	bool canRetaliate() const {
		return !state.retaliated;
	}

	bool canHeroCast() const;

	int* getReachableHexes(CombatUnit& unit) {
		// get hexes in straight line range
		// remove hexes occupied by units and solid obstacles
		// find path to hex and remove if cannot move there

		return nullptr;
	}

	float calculateUnitDamage(CombatUnit& unit, CombatUnit& target) {
		return .0f;
	}

	bool isAlive() const {
		return state.is_alive;
	}

	bool canMakeAction() const {
		return state.is_alive && !state.done;
	}

	bool canWait() const {
		return !state.waiting;
	}

	bool canDefend() const {
		return !state.defending;
	}

	// C8DE0 - get unit number shots and check if arrow tower
	bool canShoot(CombatUnit& unit) {
		return currentStats.shots > 0; // 0 && false; // unit->getNumberShots && unit->isArrowTower()
	}

	// C86D0 - check if arrow tower, ballista or 150 (cannon? undefined?)
	bool isShootingSiege(CombatUnit& unit) {
		return 0; // isBallista, isArrowTower, isCannon(150 id)?
	}

	bool isArrowTower(CombatUnit& unit) {
		return false; // creaturetype == 149
	}

	bool canFly(CombatUnit& unit) {
		return false; // flags & 2;
	}

	bool cannotMove(CombatUnit& unit) {
		return false; // flags & 0x15;
	}

	bool isDoubleWide(CombatUnit& unit) {
		return false; // flags & 1;
	}

	bool hasAtkModSpellActive() const {
		return false; // isFrenzyActive, precision, bloodlust
	}

	// 42270 - not sure
	int calcDiffAtk() const {
		if (hasAtkModSpellActive())
			;
		return currentStats.atk - unit_template.stats.atk;
	}

	// 42380 - additionaly checking if we have moat;
	int calcDiffDef() const {
		//if has moat;

		// if hasdefmodspellactive; isFrenzyActive

		return currentStats.def - unit_template.stats.def;
	}
};
