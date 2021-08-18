#include "combat_manager.h"

namespace HotaMechanics {
	using namespace Constants;

	const CombatEvent CombatManager::createFieldChangeEvent(const std::vector<uint8_t>& _hexes, const bool _destroy) const {
		return CombatEvent{ CombatEventType::FIELD_CHANGE, 0xFF, 0xFF, _destroy };
	}

	const CombatEvent CombatManager::createUnitPosChangedEvent(const int16_t _unit_id, const uint8_t _source_hex, const uint8_t _target_hex) const {
		return CombatEvent{ CombatEventType::UNIT_POS_CHANGED, _unit_id, _source_hex, _target_hex };
	}

	const CombatEvent CombatManager::createUnitHealthLostEvent(const int16_t _unit_id) const {
		return CombatEvent{ CombatEventType::UNIT_HEALTH_LOST, _unit_id, 0xFF, 0xFF };
	}

	const CombatEvent CombatManager::createUnitStatsChangedEvent(const int16_t _unit_id, const uint8_t _stats_id) const {
		return CombatEvent{ CombatEventType::UNIT_STATS_CHANGED, _unit_id, _stats_id, 0xFF };
	}

	const CombatAction CombatManager::createPreBattleAction() const {
		return CombatAction{ CombatActionType::PRE_BATTLE, 0xFF, 0xFF, false };
	}

	const CombatAction CombatManager::createPreTurnAction() const {
		return CombatAction{ CombatActionType::PRE_TURN, 0xFF, 0xFF, false };
	}

	const CombatAction CombatManager::createWaitAction() const {
		return CombatAction{ CombatActionType::WAIT, 0xFF, 0xFF, true };
	}

	const CombatAction CombatManager::createWalkAction(uint8_t _hex_id, int16_t _walk_distance) const {
		return CombatAction{ CombatActionType::WALK, _walk_distance, _hex_id, true };
	}

	const CombatAction CombatManager::createDefendAction() const {
		return CombatAction{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
	}

	const CombatAction CombatManager::createSpellCastAction(int16_t _spell_id, int16_t _unit_id, uint8_t _hex_id) const {
		throw std::exception("Not implemented yet");
	}

	const CombatAction CombatManager::createAttackAction(int16_t _unit_id, uint8_t _hex_id) const {
		return CombatAction{ CombatActionType::ATTACK, _unit_id, _hex_id, true };
	}
}; // HotaMechanics