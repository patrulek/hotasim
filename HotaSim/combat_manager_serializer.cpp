#include "combat_manager_serializer.h"

#include "../HotaMechanics/combat_manager.h"
#include "mempool.h"

#include <thread>

namespace HotaSim {
	using namespace Constants;


	CombatSerializer::CombatSerializer(CombatManager& _manager)
		: manager(_manager) {}

	std::shared_ptr<CombatStatePacked> CombatSerializer::packCombatState(const CombatState& _state) {
		auto& packed_state = Mempool::retrieveCombatStatePacked();
		auto ai = &const_cast<CombatAI&>(manager.getCombatAI());

		packed_state->last_unit = _state.last_unit;
		packed_state->turn = _state.turn;
		packed_state->result = static_cast<int8_t>(_state.result);
		packed_state->order_size = _state.order.size();

		if (packed_state->order_size > 0) {
			packed_state->order = Mempool::retrieveUint8(packed_state->order_size);
			int i = 0;
			for (auto order_unit : _state.order)
				packed_state->order[i++] = order_unit;
		}

		packed_state->attacker_stats = _state.attacker.getStats();
		packed_state->attacker_units_size = _state.attacker.getUnitsPtrs().size();

		if (packed_state->attacker_units_size > 0) {
			packed_state->attacker_units = Mempool::retrieveCombatUnitPacked(packed_state->attacker_units_size);
			int i = 0;

			for (auto unit : _state.attacker.getUnitsPtrs()) {
				packed_state->attacker_units[i].stats = unit->getStats();
				packed_state->attacker_units[i].hex = unit->getHex();
				packed_state->attacker_units[i].stack_number = unit->getStackNumber();
				packed_state->attacker_units[i].health_lost = unit->getHealthLost();
				packed_state->attacker_units[i].state = unit->getState();
				++i;
			}
		}

		packed_state->defender_stats = _state.defender.getStats();
		packed_state->defender_units_size = _state.defender.getUnitsPtrs().size();

		if (packed_state->defender_units_size > 0) {
			packed_state->defender_units = Mempool::retrieveCombatUnitPacked(packed_state->defender_units_size);
			int i = 0;

			for (auto unit : _state.defender.getUnitsPtrs()) {
				packed_state->defender_units[i].stats = unit->getStats();
				packed_state->defender_units[i].hex = unit->getHex();
				packed_state->defender_units[i].stack_number = unit->getStackNumber();
				packed_state->defender_units[i].health_lost = unit->getHealthLost();
				packed_state->defender_units[i].state = unit->getState();
				++i;
			}
		}

		for (int i = 0; i < FIELD_SIZE + 1; i+=2) {
			const int idx = i / 2;
			packed_state->hex_occupations[idx] =
				(static_cast<uint8_t>(_state.field.getById(i).getOccupation())) | (static_cast<uint8_t>(_state.field.getById(i + 1).getOccupation()) << 4);
		}

		auto& p_r = ai->getPlayerReachables();
		auto& p_a = ai->getPlayerAttackables();
		auto& ai_r = ai->getAIReachables();
		auto& ai_a = ai->getAIAttackables();

		std::copy(std::begin(p_r), std::end(p_r), std::begin(packed_state->player_reachables));
		std::copy(std::begin(p_a), std::end(p_a), std::begin(packed_state->player_attackables));
		std::copy(std::begin(ai_r), std::end(ai_r), std::begin(packed_state->ai_reachables));
		std::copy(std::begin(ai_a), std::end(ai_a), std::begin(packed_state->ai_attackables));

		return packed_state;
	}

	CombatState* CombatSerializer::unpackCombatState(const CombatStatePacked& _packed_state) {

		// attacker hero
		//CombatHero attacker_(attacker->getTemplate(), attacker->getArmyPermutation(), CombatSide::ATTACKER);
		auto current_state = &manager.getCurrentState();
		auto ai = &const_cast<CombatAI&>(manager.getCombatAI());

		auto& units = current_state->attacker.getUnitsPtrs();
		for (int i = 0; i < _packed_state.attacker_units_size; ++i) {
			const_cast<CombatUnit*>(units[i])->setStats(_packed_state.attacker_units[i].stats);
			const_cast<CombatUnit*>(units[i])->setHex(_packed_state.attacker_units[i].hex);
			const_cast<CombatUnit*>(units[i])->setStackNumber(_packed_state.attacker_units[i].stack_number);
			const_cast<CombatUnit*>(units[i])->setHealthLost(_packed_state.attacker_units[i].health_lost);
			const_cast<CombatUnit*>(units[i])->setUnitState(_packed_state.attacker_units[i].state);
		}

		// defender hero
		//CombatHero defender_(defender->getTemplate(), defender->getArmyPermutation(), CombatSide::DEFENDER);
		auto& def_units = current_state->defender.getUnitsPtrs();
		for (int i = 0; i < _packed_state.defender_units_size; ++i) {
			const_cast<CombatUnit*>(def_units[i])->setStats(_packed_state.defender_units[i].stats);
			const_cast<CombatUnit*>(def_units[i])->setHex(_packed_state.defender_units[i].hex);
			const_cast<CombatUnit*>(def_units[i])->setStackNumber(_packed_state.defender_units[i].stack_number);
			const_cast<CombatUnit*>(def_units[i])->setHealthLost(_packed_state.defender_units[i].health_lost);
			const_cast<CombatUnit*>(def_units[i])->setUnitState(_packed_state.defender_units[i].state);
		}

		// field state info
		//CombatField field_ = CombatField::retrieveCombatField(field->getType(), field->getTemplate());
		auto& field = current_state->field;
		int i = 0;
		for (auto hex_occupation : _packed_state.hex_occupations) {
			const_cast<CombatHex&>(field.getById(i++)).occupyHex(static_cast<CombatHexOccupation>(hex_occupation & 0x0F));
			const_cast<CombatHex&>(field.getById(i++)).occupyHex(static_cast<CombatHexOccupation>((hex_occupation & 0xF0) >> 4));
		}

		// ai state info
		ai->initializeBattle(&_packed_state.player_reachables, &_packed_state.player_attackables,
			&_packed_state.ai_reachables, &_packed_state.ai_attackables);

		// combat state info
		//auto state = Mempool::retrieveCombatState(std::move(attacker_), std::move(defender_), std::move(field_));
		current_state->last_unit = _packed_state.last_unit;
		current_state->turn = _packed_state.turn;
		current_state->result = static_cast<CombatResult>(_packed_state.result);
		current_state->order.clear();

		if (_packed_state.order_size > 0) {
			for (int i = 0; i < _packed_state.order_size; ++i)
				current_state->order.push_back(_packed_state.order[i]);
		}

		return current_state;
	}
}