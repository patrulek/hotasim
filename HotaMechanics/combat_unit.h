#pragma once


#include "structures.h"


struct CombatUnit {
	Unit unitTemplate{};
	uint32 currentFlags{ 0 };
	PrimaryStats currentStats;
	uint16 stackNumber{ 0 };
	uint16 currentStackNumber{ 0 };
	uint8 hexId{ 255 };

	CombatUnit(const Unit& unitTemplate)
		: unitTemplate(unitTemplate), currentStats(unitTemplate.stats), currentFlags(unitTemplate.flags) {};
	CombatUnit() = default;


	float calculateUnitFightValue(CombatUnit& unit) {
		return unit.currentStackNumber * unit.unitTemplate.fightValue;
	}

	int* getReachableHexes(CombatUnit& unit) {
		// get hexes in straight line range
		// remove hexes occupied by units and solid obstacles
		// find path to hex and remove if cannot move there

		return nullptr;
	}

	float calculateUnitDamage(CombatUnit& unit, CombatUnit& target) {
		return .0f;
	}


	// C8DE0 - get unit number shots and check if arrow tower
	bool canShoot(CombatUnit& unit) {
		return 0 && false; // unit->getNumberShots && unit->isArrowTower()
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

	bool hasAtkModSpellActive(CombatUnit& unit) {
		return false; // isFrenzyActive, precision, bloodlust
	}

	// 42270 - not sure
	int calcDiffAtk(CombatUnit& unit) {
		if (hasAtkModSpellActive(unit))
			;
		return unit.currentStats.atk - unit.unitTemplate.stats.atk;
	}

	// 42380 - additionaly checking if we have moat;
	int calcDiffDef(CombatUnit& unit) {
		//if has moat;

		// if hasdefmodspellactive; isFrenzyActive

		return unit.currentStats.def - unit.unitTemplate.stats.def;
	}
};
