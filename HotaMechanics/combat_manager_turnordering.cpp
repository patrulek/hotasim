#include "combat_manager.h"

#include <algorithm>

#include "combat_hero.h"

namespace HotaMechanics {
	using namespace Constants;


	void CombatManager::removeFromOrderList(const int _unit_id) {
		current_state->order.remove(_unit_id);
	}

	void CombatManager::restoreLastUnit() {
		current_state->order.push_front(current_state->last_unit);
		current_state->last_unit = -1;
	}

	void CombatManager::nextUnit() {
		current_state->last_unit = current_state->order.front();
		current_state->order.pop_front();
		active_stack = nullptr;
	}


	CombatUnit& CombatManager::getActiveStack() {
		if (active_stack)
			return const_cast<CombatUnit&>(*active_stack);

		if (current_state->order.empty())
			throw std::exception("Cant get active stack from empty queue");

		active_stack = &getStackByGlobalId(current_state->order.front());
		return const_cast<CombatUnit&>(*active_stack);
	}

	CombatUnit& CombatManager::getStackByGlobalId(const int _guid) const {
		auto& hero = _guid / 21 == 0 ? current_state->attacker : current_state->defender;
		return *(const_cast<CombatUnit*>(hero.getUnitsPtrs()[_guid % 21]));
	}

	CombatUnit& CombatManager::getStackByLocalId(const int _uid, const Constants::CombatSide _side) const {
		auto& hero = _side == CombatSide::ATTACKER ? current_state->attacker : current_state->defender;
		return *(const_cast<CombatUnit*>(hero.getUnitsPtrs()[_uid]));
	}

	void CombatManager::reorderUnits() {
		auto& active_stack = getActiveStack();

		if (current_state->order.size() > 2) {
			int i = 0;

			while(true) {
				auto it = std::rbegin(current_state->order);
				std::advance(it, i);
				auto it2 = std::rbegin(current_state->order);
				std::advance(it2, i + 1);

				int cur = *it, prev = *(it2);
				if (prev == current_state->order.front())
					break;

				auto& hero = prev / 21 == 0 ? current_state->attacker : current_state->defender;
				auto unit = hero.getUnitsPtrs()[prev % 21];

				if (unit->canWait())
					break;

				if (unit->getCombatStats().spd > active_stack.getCombatStats().spd)
					std::swap(*it, *it2);

				++i;
			}
		}
	}

	void CombatManager::orderUnitsInTurn()
	{
		auto& units = const_cast<std::vector<const CombatUnit*>&>(getAllUnitStacks());
		std::sort(std::begin(units), std::end(units), [](auto _obj1, auto _obj2) { return _obj1->getCombatStats().spd > _obj2->getCombatStats().spd; });

		current_state->order.clear();

		for (auto unit : units) {
			if (!unit->isAlive())
				continue;

			current_state->order.push_back(unit->getGlobalUnitId());
		}
	}

}; // HotaMechanics