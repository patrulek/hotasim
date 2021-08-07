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
	}


	CombatUnit& CombatManager::getActiveStack() const {
		if (current_state->order.empty())
			throw std::exception("Cant get active stack from empty queue");

		return getStackByGlobalId(current_state->order.front());
	}

	CombatUnit& CombatManager::getStackByGlobalId(const int _guid) const {
		auto& hero = _guid / 21 == 0 ? current_state->attacker : current_state->defender;
		return *(const_cast<CombatUnit*>(hero.getUnitsPtrs()[_guid % 21]));
	}

	CombatUnit& CombatManager::getStackByLocalId(const int _uid, const Constants::CombatSide _side) const {
		auto& hero = _side == CombatSide::ATTACKER ? current_state->attacker : current_state->defender;
		return *(const_cast<CombatUnit*>(hero.getUnitsPtrs()[_uid]));
	}

	void CombatManager::orderUnitsInTurn()
	{
		auto units = current_state->attacker.getUnits();
		auto defender_units = current_state->defender.getUnits();
		units.insert(std::end(units), std::begin(defender_units), std::end(defender_units));
		std::sort(std::begin(units), std::end(units), [](auto _obj1, auto _obj2) { return _obj1->getCombatStats().spd > _obj2->getCombatStats().spd; });

		current_state->order.clear();

		for (auto unit : units) {
			if (!unit->isAlive())
				continue;

			int offset = unit->getCombatSide() == CombatSide::ATTACKER ? 0 : 21;
			current_state->order.push_back(unit->getUnitId() + offset);
		}
	}

}; // HotaMechanics