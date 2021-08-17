#pragma once

#include <vector>

namespace HotaMechanics {
	class CombatManager;
	struct CombatAction;
}

namespace HotaSim {
	class CombatRewinder
	{
	public:
		CombatRewinder(HotaMechanics::CombatManager& _manager)
			: manager(_manager) {}

		void describeAction(const HotaMechanics::CombatAction& _action);
		void rewind(const std::vector<int>& _actions, const std::vector<int>& _seeds);

	private:
		HotaMechanics::CombatManager& manager;
	};
}

