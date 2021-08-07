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

	int getRandomInt(int _min, int _max);
	float getRandomFloat(float _min, float _max);

	std::vector<int> getCombatFieldTemplate(const Constants::CombatFieldTemplate _field_template);


	template<typename Printable>
	std::string to_string(const Printable& _printable) {
		//if (std::is_fundamental_v<Printable>)
		//	return std::to_string((std::remove_reference_t<Printable>)_printable);
		//else
			return _printable.toString();
	}

}; // HotaMechanics::Utils
