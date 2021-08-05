#pragma once

#include "../HotaMechanics/structures.h"

namespace HotaSim::Utils {
	using namespace HotaMechanics;

	// TODO
	struct HeroCreator : public Hero {
		void setAttack(const int _attack) { stats.base_stats.atk = _attack; }
		void setDefense(const int _defense) { stats.base_stats.def = _defense; }
		void setPower(const int _power) { stats.base_stats.pow = _power; }
		void setKnowledge(const int _knowledge) {
			stats.base_stats.kgd = _knowledge;
			stats.primary_stats.mana = _knowledge * 10;
		}

		void setHeroArmy(const std::vector<UnitStack>& _army) {
			army = _army;
		}

		void addUnit(const Unit& _unit, const int16_t _stack_size) {
			addUnitStack(UnitStack{ _unit, _stack_size });
		}

		void addUnitStack(const UnitStack& _unit_stack) {
			if (army.size() >= MAX_ARMY_SIZE)
				return;
			army.push_back(_unit_stack);
		}

		void removeHeroArmy() { army.clear(); }
	};
} // HotaSim::Utils