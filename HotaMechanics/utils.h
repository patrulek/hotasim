#pragma once

#include <vector>
#include <string>

#include "structures.h"

namespace HotaMechanics {
	class CombatUnit;
};

namespace HotaMechanics::Utils {
	BaseStats baseStats(int _atk, int _def, int _pow, int _kgd);
	CombatStats combatStats(int _min, int _max, int _spd, int _shots);
	PrimaryStats primaryStats(int _hp, int _mana);

	std::string to_string(const CombatUnit& _unit);

	int getRandomInt(int _min, int _max);
	float getRandomFloat(float _min, float _max);

	std::vector<int> getCombatFieldTemplate(const Constants::CombatFieldTemplate _field_template);

}; // HotaMechanics::Utils
