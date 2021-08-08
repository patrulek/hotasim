#pragma once

#include <vector>

namespace HotaMechanics {
	struct CombatState;
	struct CombatAction;
	class CombatManager;
}

namespace HotaSim {
	namespace Estimator
	{
		const int estimateTurnsNumber(const HotaMechanics::CombatState& _initial_state);
		const int estimateTotalStatesNumber(const HotaMechanics::CombatState& _initial_state);

		const std::vector<int> shuffleActions(const std::vector<HotaMechanics::CombatAction>& _actions, const HotaMechanics::CombatManager& _manager, const int _seed);
	}; // HotaSim::Estimator
} // HotaSim

