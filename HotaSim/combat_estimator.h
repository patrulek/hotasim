#pragma once

namespace HotaMechanics {
	struct CombatState;
}

namespace HotaSim {
	namespace Estimator
	{
		const int estimateTurnsNumber(const HotaMechanics::CombatState& _initial_state);
		const int estimateTotalStatesNumber(const HotaMechanics::CombatState& _initial_state);
	}; // HotaSim::Estimator
} // HotaSim

