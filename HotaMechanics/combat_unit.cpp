#include "combat_unit.h"

#include "combat_field.h"
#include "combat_hero.h"

const std::vector<int> CombatUnit::getHexesInSpeedRange(const CombatField& field) const {
	return field.getHexesInRange(hexId, currentStats.spd);
}

const std::vector<int> CombatUnit::getHexesInAttackRange(const CombatField& field) const {
	return field.getHexesInRange(hexId, currentStats.spd + 1);
}

int CombatUnit::getUnitId() const {
	if (hero)
		return hero->getUnitId(*this);
	return -1;
}

CombatSide CombatUnit::getCombatSide() const {
	if (hero) {
		// todo: check if hypnotized
		return hero->getCombatSide();
	}
	return CombatSide::ATTACKER;
}


bool CombatUnit::canHeroCast() const {
	if (hero)
		return hero->canCast();
	return false;
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