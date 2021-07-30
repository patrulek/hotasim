#include "combat_unit.h"

#include "combat_field.h"

const std::vector<int> CombatUnit::getHexesInSpeedRange(const CombatField& field) const {
	return field.getHexesInRange(hexId, currentStats.spd);
}