#include "combat_unit.h"

#include "combat_field.h"
#include "combat_hero.h"

void CombatUnit::applyHeroStats() {
	applyHeroStats(hero->getStats());
}

void CombatUnit::moveTo(const int _target_hex) {
	hex = _target_hex;
}

void CombatUnit::walkTo(const int _target_hex) {
	throw std::exception("Not implemented yet");
}

int CombatUnit::getUnitId() const {
	return hero->getUnitId(*this);
}

CombatSide CombatUnit::getCombatSide() const {
	// todo: check if hypnotized
	return hero->getCombatSide();
}

void CombatUnit::defend() {
	state.defending = true;
	state.done = true;
}

void CombatUnit::wait() {
	state.waiting = true;
}

void CombatUnit::resetState() {
	state.defending = state.done = state.morale = state.retaliated = state.waiting = false;
}


bool CombatUnit::canHeroCast() const {
	return hero->canCast();
}


float CombatUnit::getBaseAverageDmg() const {
	return (unit_template.stats.min_dmg + unit_template.stats.max_dmg) / 2.0f;
}

void CombatUnit::applyDamage(int damage) {
	if (damage == 0)
		return;

	if (damage >= calculateStackHP()) {
		state.is_alive = false;
		stackNumber = health_lost = 0;
		return;
	}

	int total_health_after_dmg = calculateStackHP() - damage;
	health_lost = (currentStats.hp - total_health_after_dmg % currentStats.hp) % currentStats.hp;
	stackNumber = (total_health_after_dmg / currentStats.hp) + (health_lost != 0);
}