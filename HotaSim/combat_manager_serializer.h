#pragma once

#include <memory>

namespace HotaMechanics {
	class CombatManager;
	struct CombatState;
	struct CombatStatePacked;
}

namespace HotaSim {
	using namespace HotaMechanics;

	struct CombatSerializer {
		CombatSerializer(CombatManager& _manager);

		// state serializer -------------
		std::shared_ptr<CombatStatePacked> packCombatState(const CombatState& _state);
		CombatState* unpackCombatState(const CombatStatePacked& _packed_state);
		// ------------------------------

		CombatManager& manager;
	};
}