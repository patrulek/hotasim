#pragma once

#include "../HotaMechanics/structures.h"

namespace HotaSim::Utils {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Constants;

	// TODO
	struct HeroCreator {
		Hero hero;

		void setAttack(const int _attack) { hero.stats.base_stats.atk = _attack; }
		void setDefense(const int _defense) { hero.stats.base_stats.def = _defense; }
		void setPower(const int _power) { hero.stats.base_stats.pow = _power; }
		void setKnowledge(const int _knowledge) {
			hero.stats.base_stats.kgd = _knowledge;
			hero.stats.primary_stats.mana = _knowledge * 10;
		}

		void setHeroArmy(const std::vector<UnitStack>& _army) {
			hero.army = _army;
		}

		void addUnit(const Unit& _unit, const int16_t _stack_size) {
			addUnitStack(UnitStack{ _unit, _stack_size });
		}

		void addUnitStack(const UnitStack& _unit_stack) {
			if (hero.army.size() >= MAX_ARMY_SIZE)
				return;
			hero.army.push_back(_unit_stack);
		}

		void removeHeroArmy() { hero.army.clear(); }
	};
} // HotaSim::Utils