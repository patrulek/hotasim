#include "combat_manager.h"

namespace HotaMechanics {
	using namespace Constants;

	const CombatAction CombatManager::createPreBattleAction() const {
		return CombatAction{ CombatActionType::PRE_BATTLE, -1, -1, false };
	}

	const CombatAction CombatManager::createPreTurnAction() const {
		return CombatAction{ CombatActionType::PRE_TURN, -1, -1, false };
	}

	const CombatAction CombatManager::createWaitAction() const {
		return CombatAction{ CombatActionType::WAIT, -1, -1, true };
	}

	const CombatAction CombatManager::createWalkAction(int16_t _hex_id, int16_t _walk_distance) const {
		return CombatAction{ CombatActionType::WALK, _walk_distance, _hex_id, true };
	}

	const CombatAction CombatManager::createDefendAction() const {
		return CombatAction{ CombatActionType::DEFENSE, -1, -1, true };
	}

	const CombatAction CombatManager::createSpellCastAction(int16_t _spell_id, int16_t _unit_id, int16_t _hex_id) const {
		throw std::exception("Not implemented yet");
	}

	const CombatAction CombatManager::createAttackAction(int16_t _unit_id, int16_t _hex_id) const {
		return CombatAction{ CombatActionType::ATTACK, _unit_id, _hex_id, true };
	}
}; // HotaMechanics