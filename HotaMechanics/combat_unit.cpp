#include "combat_unit.h"

#include "combat_field.h"
#include "combat_hero.h"

namespace HotaMechanics {
	using namespace Constants;


	CombatUnit::CombatUnit(const Unit& _unit_template, const CombatHero& _hero, const int16_t _stack_number)
		: unit_template(&_unit_template), hero(&_hero), stack_number(_stack_number) {
		uid = (UnitId)getHero().getUnitsPtrs().size();
		stats = _unit_template.stats;
	}

	CombatUnit::CombatUnit(const CombatUnit& _unit, const CombatHero& _hero)
		: unit_template(_unit.unit_template), hero(&_hero) {
		uid = _unit.uid;
		state = _unit.state;
		hex = _unit.hex;
		stats = _unit.stats;
		stack_number = _unit.stack_number;
		health_lost = _unit.health_lost;
		active_spells = _unit.active_spells;
	}

	CombatUnit::CombatUnit(CombatUnit&& _unit, const CombatHero& _hero)
		: unit_template(_unit.unit_template), hero(&_hero) {
		uid = std::move(_unit.uid);
		state = std::move(_unit.state);
		hex = std::move(_unit.hex);
		stats = std::move(_unit.stats);
		stack_number = std::move(_unit.stack_number);
		health_lost = std::move(_unit.health_lost);
		active_spells = std::move(_unit.active_spells);
	}

	std::string CombatUnit::toString() const {
		return getTemplate().name + "(" + std::to_string(uid) + ") : StackNumber(" + std::to_string(stack_number) + ") : Hex(" + std::to_string(hex / FIELD_COLS) + ", " 
			+ std::to_string(hex % FIELD_COLS) + ") : " + (getCombatSide() == CombatSide::ATTACKER ? "player_unit" : "ai_unit");
	}

	void CombatUnit::moveTo(const HexId _target_hex) {
		hex = _target_hex;
	}

	void CombatUnit::defend() {
		state.flags.defending = true;
		state.flags.done = true;
	}

	void CombatUnit::wait() {
		state.flags.waiting = true;
	}

	void CombatUnit::applyDamage(const int _damage) {
		if (_damage == 0)
			return;

		if (_damage >= getUnitStackHP()) {
			state.flags.is_alive = false;
			stack_number = health_lost = 0;
			return;
		}

		const int total_health_after_dmg = getUnitStackHP() - _damage;
		health_lost = (stats.primary_stats.hp - total_health_after_dmg % stats.primary_stats.hp) % stats.primary_stats.hp;
		stack_number = (total_health_after_dmg / stats.primary_stats.hp) + (health_lost != 0);
	}

	void CombatUnit::resetState() {
		state.flags.defending = state.flags.done = state.flags.morale = state.flags.retaliated = state.flags.waiting = false;
	}

	void CombatUnit::initUnit() {
		state.flags.is_alive = true;
	}

	void CombatUnit::applyHeroStats() {
		BaseStats hero_stats = getHero().getBaseStats();
		stats.base_stats.atk += (hero_stats.atk * !state.flags.applied_hero_stats);
		stats.base_stats.def += (hero_stats.def * !state.flags.applied_hero_stats);
		state.flags.applied_hero_stats = true;
	}

	void CombatUnit::setDone() {
		state.flags.done = true;
	}

	void CombatUnit::unsetDone() {
		state.flags.done = false;
	}

	void CombatUnit::setRetaliated() {
		state.flags.retaliated = true;
	}

	bool CombatUnit::canHeroCast() const {
		return getHero().canCast();
	}

	const int16_t CombatUnit::getAttackGain() const {
		return stats.base_stats.atk - getTemplate().stats.base_stats.atk;
	}

	const int16_t CombatUnit::getDefenseGain() const {
		const int bonus_def = state.flags.defending;
		//if has moat;
		// if hasdefmodspellactive; isFrenzyActive
		return stats.base_stats.def + bonus_def - getTemplate().stats.base_stats.def;
	}
	const float CombatUnit::getBaseAverageDmg() const {
		return (getTemplate().stats.combat_stats.min_dmg + getTemplate().stats.combat_stats.max_dmg) / 2.0f;
	}

	const float CombatUnit::getFightValuePerOneHp() const {
		return getFightValue() / stats.primary_stats.hp;
	}

	const float CombatUnit::getFightValuePerUnitStack() const {
		return getUnitStackHP() * getFightValuePerOneHp();
	}

	const int CombatUnit::getUnitStackHP() const {
		return stack_number * stats.primary_stats.hp - health_lost;
	}

	const UnitId CombatUnit::getGlobalUnitId() const {
		return getCombatSide() == CombatSide::ATTACKER ? uid : uid + GUID_OFFSET;
	}

	const CombatSide CombatUnit::getCombatSide() const {
		return getHero().getCombatSide();
	}

	const CombatSide CombatUnit::getEnemyCombatSide() const {
		return getCombatSide() == CombatSide::ATTACKER ? CombatSide::DEFENDER : CombatSide::ATTACKER;
	}

	Hash CombatUnit::rehash() {
		Hash state_hash = state.flagsToValue();
		Hash hash = getGlobalUnitId() | (hex << 6) | (stack_number << 12) | (health_lost << 28)
			| (state_hash << 38);
		
		hash = std::hash<Hash>{}(hash);
		hash ^= std::hash<Hash>{}(stats.base_stats.stats);
		hash ^= std::hash<Hash>{}(stats.combat_stats.stats);
		hash ^= std::hash<Hash>{}(stats.primary_stats.stats);
		
		return hash;
	}

} // HotaMechanics