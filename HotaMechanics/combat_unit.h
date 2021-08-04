#pragma once


#include "structures.h"

#include <string>
#include <array>
#include <vector>

class CombatHex;
class CombatHero;
class CombatField;
enum class CombatSide;


struct UnitState {
	bool is_alive;
	bool is_summon;
	bool is_clone;
	bool morale;
	bool waiting;
	bool done;
	bool defending;
	bool sacrificed;
	bool retaliated;
};

class CombatUnit {
private:
	int hex{ -1 };
public:
	Unit unit_template{};
	UnitState state{ 0 };
	PrimaryStats currentStats;
	uint16 stackNumber{ 0 };
	uint16 health_lost{ 0 };
	std::array<SpellID, 8> active_spells{};
	bool applied_hero_stats{ false };

	const CombatHero* hero;

	explicit CombatUnit(const Unit& _unit_template, const int _stack_number, const CombatHero& _hero)
		: unit_template(_unit_template), currentStats(_unit_template.stats), stackNumber(_stack_number), hero(&_hero) {};

	explicit CombatUnit(const CombatUnit& _unit, const CombatHero& _hero)
		: hero(&_hero) {
		unit_template = _unit.unit_template;
		state = _unit.state;
		currentStats = _unit.currentStats;
		stackNumber = _unit.stackNumber;
		health_lost = _unit.health_lost;
		active_spells = _unit.active_spells;
		applied_hero_stats = _unit.applied_hero_stats;
	}
	
	CombatUnit() = delete;

	CombatSide getCombatSide() const;

	std::string to_string() {
		return unit_template.name + " : " + std::to_string(stackNumber) + " : " + std::to_string(hex) + " : "
			+ (getCombatSide() == CombatSide::ATTACKER ? "player_unit" : "ai_unit");
	}

	void moveTo(const int _target_hex);
	void walkTo(const int _target_hex);
	int getHex() const { return hex; };

	int getUnitId() const;

	void applyDamage(int damage);

	float getBaseAverageDmg() const;

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

	void applyHeroStats();

	void initUnit() {
		state.is_alive = true;
	}

	bool isDoubleWide() const {
		return false; // TODO
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
