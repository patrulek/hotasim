#include "combat_manager.h"


namespace HotaMechanics {
	using namespace Constants;

	std::shared_ptr<CombatStatePacked> CombatManager::packCombatState(const CombatState& _state) {
		auto packed_state = std::make_shared<CombatStatePacked>();

		packed_state->last_unit = _state.last_unit;
		packed_state->turn = _state.turn;
		packed_state->result = static_cast<int16_t>(_state.result);
		packed_state->order_size = _state.order.size();

		if (packed_state->order_size > 0) {
			packed_state->order = new int16_t[packed_state->order_size];
			int i = 0;
			for (auto order_unit : _state.order)
				packed_state->order[i++] = order_unit;
		}

		packed_state->attacker_stats = _state.attacker.getStats();
		packed_state->attacker_units_size = _state.attacker.getUnitsPtrs().size();

		if (packed_state->attacker_units_size > 0) {
			packed_state->attacker_units = new CombatUnitPacked[packed_state->attacker_units_size];
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
			packed_state->defender_units = new CombatUnitPacked[packed_state->defender_units_size];
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

		for (int i = 0; i < FIELD_SIZE + 1; ++i) {
			const int idx = i / 2;
			const int shift = (i % 2 == 1) * 4;
			packed_state->hex_occupations[idx] |= (static_cast<int8_t>(_state.field.getById(i).getOccupation()) << shift);
		}

		for (int i = 0; i < FIELD_SIZE + 1; ++i) {
			packed_state->player_reachables[i] = static_cast<int8_t>(ai->getPlayerReachables()[i]);
			packed_state->player_attackables[i] = static_cast<int8_t>(ai->getPlayerAttackables()[i]);
			packed_state->ai_reachables[i] = static_cast<int8_t>(ai->getAIReachables()[i]);
			packed_state->ai_attackables[i] = static_cast<int8_t>(ai->getAIAttackables()[i]);
		}

		return packed_state;
	}

	std::shared_ptr<CombatState> CombatManager::unpackCombatState(const CombatStatePacked& _packed_state) {

		// attacker hero
		CombatHero attacker_(attacker->getTemplate(), attacker->getArmyPermutation(), CombatSide::ATTACKER);
		auto units = attacker_.getUnitsPtrs();
		for (int i = 0; i < _packed_state.attacker_units_size; ++i) {
			const_cast<CombatUnit*>(units[i])->setStats(_packed_state.attacker_units[i].stats);
			const_cast<CombatUnit*>(units[i])->setHex(_packed_state.attacker_units[i].hex);
			const_cast<CombatUnit*>(units[i])->setStackNumber(_packed_state.attacker_units[i].stack_number);
			const_cast<CombatUnit*>(units[i])->setHealthLost(_packed_state.attacker_units[i].health_lost);
			const_cast<CombatUnit*>(units[i])->setUnitState(_packed_state.attacker_units[i].state);
		}

		// defender hero
		CombatHero defender_(defender->getTemplate(), defender->getArmyPermutation(), CombatSide::DEFENDER);
		units = defender_.getUnitsPtrs();
		for (int i = 0; i < _packed_state.defender_units_size; ++i) {
			const_cast<CombatUnit*>(units[i])->setStats(_packed_state.defender_units[i].stats);
			const_cast<CombatUnit*>(units[i])->setHex(_packed_state.defender_units[i].hex);
			const_cast<CombatUnit*>(units[i])->setStackNumber(_packed_state.defender_units[i].stack_number);
			const_cast<CombatUnit*>(units[i])->setHealthLost(_packed_state.defender_units[i].health_lost);
			const_cast<CombatUnit*>(units[i])->setUnitState(_packed_state.defender_units[i].state);
		}

		// field state info
		CombatField field_(field->getType(), field->getTemplate());
		int i = 0;
		for ( auto hex_occupation : _packed_state.hex_occupations) {
			const_cast<CombatHex&>(field_.getById(i++)).occupyHex(static_cast<CombatHexOccupation>(hex_occupation & 0x0F));
			const_cast<CombatHex&>(field_.getById(i++)).occupyHex(static_cast<CombatHexOccupation>((hex_occupation & 0xF0) >> 4));
		}

		bool h1 = field_.getById(88).getOccupation() == CombatHexOccupation::UNIT;
		bool h2 = field_.getById(94).getOccupation() == CombatHexOccupation::UNIT;
		// ai state info
		FieldArray player_reachables, player_attackables, ai_reachables, ai_attackables;
		for (int i = 0; i < FIELD_SIZE + 1; ++i) {
			player_reachables[i] = _packed_state.player_reachables[i];
			player_attackables[i] = _packed_state.player_attackables[i];
			ai_reachables[i] = _packed_state.ai_reachables[i];
			ai_attackables[i] = _packed_state.ai_attackables[i];
		}

		ai->initializeBattle(&player_reachables, &player_attackables, &ai_reachables, &ai_attackables);

		// combat state info
		auto state = std::make_shared<CombatState>(attacker_, defender_, field_);
		state->last_unit = _packed_state.last_unit;
		state->turn = _packed_state.turn;
		state->result = static_cast<CombatResult>(_packed_state.result);

		if (_packed_state.order_size > 0) {
			for (int i = 0; i < _packed_state.order_size; ++i)
				state->order.push_back(_packed_state.order[i]);
		}

		return state;
	}
}