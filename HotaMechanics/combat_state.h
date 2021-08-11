#pragma once

#include "combat_hero.h"
#include "combat_field.h"

#include "utils.h"
#include <memory>

namespace HotaMechanics {
	class CombatAI;

	struct CombatState {
		explicit CombatState(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field)
			: attacker(_attacker), defender(_defender), field(_field) {}
		CombatState() = delete;

		int16_t turn{ -1 };
		int16_t last_unit{ -1 };
		Constants::CombatResult result{ Constants::CombatResult::NOT_STARTED };

		CombatHero attacker;
		CombatHero defender;
		CombatField field;

		std::list<int16_t> order;	// for attacker it will be 0-20; for defender it will be 21-41
	};

	struct CombatUnitPacked {
		UnitStats stats{ 0 };
		uint16_t hex{ 0 };
		uint16_t stack_number{ 0 };
		uint16_t health_lost{ 0 };
		CombatUnitState state{ 0 };
	};

	struct CombatStatePacked {
		~CombatStatePacked() {
			if (order) delete[] order;
			if (attacker_units) delete[] attacker_units;
			if (defender_units) delete[] defender_units;
		}

		// combat state
		int16_t last_unit : 6;
		int16_t turn : 8;
		int16_t result : 3;
		int16_t order_size : 6;
		int16_t* order{ nullptr };

		// attacker state
		HeroStats attacker_stats{ 0 };
		uint16_t attacker_units_size : 3;
		CombatUnitPacked* attacker_units{ nullptr };

		// defender state
		HeroStats defender_stats{ 0 };
		uint16_t defender_units_size : 3;
		CombatUnitPacked* defender_units{ nullptr };

		// field state
		static constexpr size_t HEX_OCCUPATION_SIZE = Utils::ceil((Constants::FIELD_SIZE + 1) / 2.0f);
		std::array<uint8_t, HEX_OCCUPATION_SIZE> hex_occupations; // 4 bity na jeden hex -> 

		// ai state
		std::array<uint8_t, Constants::FIELD_SIZE + 1> player_reachables; // 4 bity, na razie i tak wiêcej jednostek nie mo¿emy
		std::array<uint8_t, Constants::FIELD_SIZE + 1> player_attackables;
		std::array<uint8_t, Constants::FIELD_SIZE + 1> ai_reachables;
		std::array<uint8_t, Constants::FIELD_SIZE + 1> ai_attackables;
	};


}; // HotaMechanics