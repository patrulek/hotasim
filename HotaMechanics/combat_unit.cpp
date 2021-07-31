#include "combat_unit.h"

#include "combat_field.h"
#include "combat_hero.h"

const std::vector<int> CombatUnit::getHexesInSpeedRange(const CombatField& field) const {
	return field.getHexesInRange(hexId, currentStats.spd);
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