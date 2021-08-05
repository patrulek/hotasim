#pragma once

#include <vector>
#include <string>

#include "structures.h"

namespace HotaMechanics {
	class CombatUnit;
}

namespace HotaMechanics::Utils {

	std::string to_string(const CombatUnit& _unit);

	int getRandomInt(int _min, int _max);
	float getRandomFloat(float _min, float _max);

	std::vector<int> getCombatFieldTemplate(const Constants::CombatFieldTemplate _field_template);

} // HotaMechanics::Utils
