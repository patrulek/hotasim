#include "combat_unit.h"

#include "combat_field.h"
#include "combat_hero.h"

namespace HotaMechanics {
	using namespace Constants;

	CombatUnit::CombatUnit(const Unit& _unit_template, const int _stack_number, const CombatHero& _hero)
		: unit_template(_unit_template), stats(_unit_template.stats), stack_number(_stack_number), hero(&_hero) {};

	CombatUnit::CombatUnit(const CombatUnit& _unit, const CombatHero& _hero)
		: hero(&_hero), unit_template(_unit.unit_template) {
		state = _unit.state;
		hex = _unit.hex;
		stats = _unit.stats;
		stack_number = _unit.stack_number;
		health_lost = _unit.health_lost;
		active_spells = _unit.active_spells;
	}

	CombatUnit::CombatUnit(CombatUnit&& _unit, const CombatHero& _hero)
		: hero(&_hero), unit_template(std::move(_unit.unit_template)) {
		state = std::move(_unit.state);
		hex = std::move(_unit.hex);
		stats = std::move(_unit.stats);
		stack_number = std::move(_unit.stack_number);
		health_lost = std::move(_unit.health_lost);
		active_spells = std::move(_unit.active_spells);
	}

	void CombatUnit::moveTo(const int _target_hex) {
		hex = _target_hex;
	}

	void CombatUnit::defend() {
		state.defending = true;
		state.done = true;
	}

	void CombatUnit::wait() {
		state.waiting = true;
	}

	void CombatUnit::applyDamage(const int _damage) {
		if (_damage == 0)
			return;

		if (_damage >= getUnitStackHP()) {
			state.is_alive = false;
			stack_number = health_lost = 0;
			return;
		}

		int total_health_after_dmg = getUnitStackHP() - _damage;
		health_lost = (stats.primary_stats.hp - total_health_after_dmg % stats.primary_stats.hp) % stats.primary_stats.hp;
		stack_number = (total_health_after_dmg / stats.primary_stats.hp) + (health_lost != 0);
	}

	void CombatUnit::resetState() {
		state.defending = state.done = state.morale = state.retaliated = state.waiting = false;
	}

	void CombatUnit::initUnit() {
		state.is_alive = true;
	}

	void CombatUnit::applyHeroStats() {
		BaseStats hero_stats = hero->getBaseStats();
		stats.base_stats.atk += (hero_stats.atk * !state.applied_hero_stats);
		stats.base_stats.def += (hero_stats.def * !state.applied_hero_stats);
		state.applied_hero_stats = true;
	}

	void CombatUnit::setDone() {
		state.done = true;
	}

	void CombatUnit::unsetDone() {
		state.done = false;
	}

	void CombatUnit::setRetaliated() {
		state.retaliated = true;
	}

	bool CombatUnit::canHeroCast() const {
		return hero->canCast();
	}

	const int CombatUnit::getAttackGain() const {
		return stats.base_stats.atk - unit_template.stats.base_stats.atk;
	}

	const int CombatUnit::getDefenseGain() const {
		int bonus_def = state.defending;
		//if has moat;
		// if hasdefmodspellactive; isFrenzyActive
		return stats.base_stats.def + bonus_def - unit_template.stats.base_stats.def;
	}
	const float CombatUnit::getBaseAverageDmg() const {
		return (unit_template.stats.combat_stats.min_dmg + unit_template.stats.combat_stats.max_dmg) / 2.0f;
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

	const int CombatUnit::getUnitId() const {
		return hero->getUnitId(*this);
	}

	const CombatSide CombatUnit::getCombatSide() const {
		return hero->getCombatSide();
	}

} // HotaMechanics