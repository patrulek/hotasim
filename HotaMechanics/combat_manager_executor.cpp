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

		reorderUnits();
	}

	void CombatManager::makeUnitFly(int _target_hex) {
		throw std::exception("Not implemented yet");
	}

	void CombatManager::makeUnitWalk(int _target_hex, int _walk_distance) {
		auto& active_stack = getActiveStack();
		auto path = const_cast<CombatPathfinder&>(ai->getPathfinder()).findPath(active_stack.getHex(), _target_hex, current_state->field);

		if (path.empty()) {
			path = const_cast<CombatPathfinder&>(ai->getPathfinder()).findPath(active_stack.getHex(), _target_hex, current_state->field);
			this->generateActionsForPlayer();
			throw std::exception("Should never happen (we already found that path earlier)");
		}

		auto new_event = createUnitPosChangedEvent(active_stack.getGlobalUnitId(), active_stack.getHex(), active_stack.getHex());
		int walk_distance = _walk_distance == -1 ? active_stack.getCombatStats().spd : _walk_distance;
		int range = std::min(path.size(), (size_t)walk_distance);
		for (int i = 0; i < range; ++i) {
			moveUnit(active_stack, path[i]);
			new_event.param3 = path[i];
		}

		action_events.push_back(new_event);
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

		auto& defender = getStackByLocalId(_unit_id, active_stack.getEnemyCombatSide());
		int dmg = calculateMeleeUnitAverageDamage(active_stack, defender);
		int counter_dmg = calculateCounterAttackMeleeUnitAverageDamage(active_stack, defender);


		//std::cout << "(" << active_stack.to_string() << ") attack (" << defender->to_string() << ")\n";
		defender.applyDamage(dmg);
		action_events.push_back(createUnitHealthLostEvent(defender.getGlobalUnitId()));
		//std::cout << "(" << active_stack.to_string() << ") dealt " << dmg << " to (" << defender->to_string() << ")\n";

		if (defender.isAlive()) {
			if (defender.canRetaliate()) {
				//std::cout << "(" << defender->to_string() << ") counter attack (" << active_stack.to_string() << ")\n";
				active_stack.applyDamage(counter_dmg);
				action_events.push_back(createUnitHealthLostEvent(active_stack.getGlobalUnitId()));
				//std::cout << "(" << defender->to_string() << ") counter attack dealt " << counter_dmg << " to (" << active_stack.to_string() << ")\n";
				defender.setRetaliated();
			}
		}
		else {
			removeFromOrderList(defender.getGlobalUnitId());
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