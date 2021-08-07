#include "combat_estimator.h"

#include "../HotaMechanics/structures.h"

using namespace HotaMechanics;

namespace HotaSim {
	namespace Estimator
	{
		const int estimateTurnsNumber(const CombatState& _initial_state) {
			return 3; // TODO: implement
		}

		const int estimateTotalStatesNumber(const CombatState& _initial_state) {
			return 500000; // TODO: implement
		}
	}; // HotaSim::Estimator
} // HotaSim

