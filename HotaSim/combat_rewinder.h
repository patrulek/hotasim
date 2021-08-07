#pragma once

#include <vector>

namespace HotaMechanics {
	class CombatManager;
	class CombatAction;
}

namespace HotaSim {
	class CombatRewinder
	{
	public:
		CombatRewinder(HotaMechanics::CombatManager& _manager)
			: manager(_manager) {}

		void describeAction(const HotaMechanics::CombatAction& _action);
		void rewind(const std::vector<int>& _actions);

	private:
		HotaMechanics::CombatManager& manager;
	};
}

