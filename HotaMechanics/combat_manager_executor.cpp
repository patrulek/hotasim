#include "combat_manager.h"

#include "combat_calculator.h"

namespace HotaMechanics {
	using namespace Constants;
	using namespace Calculator;

	void CombatManager::makeUnitDefend() {
		auto& active_stack = getActiveStack();
		active_stack.defend();
	}

	void CombatManager::makeUnitWait() {
		auto& active_stack = getActiveStack();
		active_stack.wait();
		current_state->order.push_back(current_state->order.front());

		if (current_state->order.size() > 2) {
			for (auto it = std::rbegin(current_state->order), it2 = it; ; ++it) {
				std::advance(it2, 1);
				int cur = *it, prev = *(it2);
				auto& hero = prev / 21 == 0 ? current_state->attacker : current_state->defender;
				auto unit = hero.getUnits()[prev % 21];

				if (unit->canWait())
					break;

				if (!(unit->getCombatStats().spd > active_stack.getCombatStats().spd)) {
					std::swap(*it, *it2);
				}
			}
		}
	}

	void CombatManager::makeUnitFly(int _target_hex) {
		throw std::exception("Not implemented yet");
	}

	void CombatManager::makeUnitWalk(int _target_hex, int _walk_distance) {
		auto& active_stack = getActiveStack();
		auto path = ai->getPathfinder().findPath(active_stack.getHex(), _target_hex, current_state->field);

		if (path.empty())
			throw std::exception("Should never happen (we already found that path earlier)");

		int walk_distance = _walk_distance == -1 ? active_stack.getCombatStats().spd : _walk_distance;
		int range = std::min(path.size(), (size_t)walk_distance);
		for (int i = 0; i < range; ++i) {
			//std::cout << "Moved unit from (" << active_stack.getHex() << ") to (" << path[i] << ")\n";
			moveUnit(active_stack, path[i]);
		}

		active_stack.setDone(); // TODO: shouldnt be done yet if we're attacking, unless quicksand or smth
	}

	void CombatManager::makeUnitAttack(int _unit_id, int _target_hex) {
		auto& active_stack = getActiveStack();
		if (active_stack.getHex() != _target_hex) {
			makeUnitWalk(_target_hex);
			active_stack.unsetDone(); // TODO: need to fix it somehow
		}

		if (!active_stack.isAlive())
			return;

		auto defender = active_stack.getCombatSide() == CombatSide::ATTACKER ? current_state->defender.getUnits()[_unit_id] : current_state->attacker.getUnits()[_unit_id];
		int dmg = calculateMeleeUnitAverageDamage(active_stack, *defender);
		int counter_dmg = calculateCounterAttackMeleeUnitAverageDamage(active_stack, *defender);


		//std::cout << "(" << active_stack.to_string() << ") attack (" << defender->to_string() << ")\n";
		const_cast<CombatUnit*>(defender)->applyDamage(dmg);
		//std::cout << "(" << active_stack.to_string() << ") dealt " << dmg << " to (" << defender->to_string() << ")\n";

		if (defender->isAlive()) {
			if (defender->canRetaliate()) {
				//std::cout << "(" << defender->to_string() << ") counter attack (" << active_stack.to_string() << ")\n";
				active_stack.applyDamage(counter_dmg);
				//std::cout << "(" << defender->to_string() << ") counter attack dealt " << counter_dmg << " to (" << active_stack.to_string() << ")\n";
				const_cast<CombatUnit*>(defender)->setRetaliated();
			}
		}
		else {
			removeFromOrderList(_unit_id + (defender->getCombatSide() == CombatSide::ATTACKER ? 0 : 21));
		}

		if (active_stack.isAlive())
			active_stack.setDone();
	}

	void CombatManager::moveUnit(CombatUnit& _unit, int _target_hex) {
		current_state->field.clearHex(_unit.getHex());
		// TODO: check double_wide
		_unit.moveTo(_target_hex);
		current_state->field.fillHex(_target_hex, CombatHexOccupation::UNIT);
		// TODO: check double_wide
	}
}; // HotaMechanics