#pragma once

#include "combat_hero.h"
#include "combat_field.h"
#include "utils.h"
#include <memory>
#include <iostream>
#include <utility>

namespace HotaMechanics {
	class CombatAI;

	struct CombatState {
		CombatState(CombatHero&& _attacker, CombatHero&& _defender, CombatField&& _field) noexcept
			: attacker(std::move(_attacker)), defender(std::move(_defender)), field(std::move(_field)) {
		}
		CombatState(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field)
			: attacker(_attacker), defender(_defender), field(_field) {}
		CombatState() = delete;
		~CombatState();

		int16_t turn{ -1 };
		UnitId last_unit{ Constants::INVALID_UNIT_ID };
		Constants::CombatResult result{ Constants::CombatResult::NOT_STARTED };

		CombatHero attacker;
		CombatHero defender;
		CombatField field;

		std::list<UnitId> order;	// for attacker it will be 0-20; for defender it will be 21-41
		Hash rehash();
	};

	struct CombatUnitPacked {
		UnitStats stats{ 0 };
		uint8_t hex{ 0 };
		uint16_t stack_number{ 0 };
		uint16_t health_lost{ 0 };
		CombatUnitState state{ 0 };
	};

	struct CombatStatePacked {
		// combat state
		UnitId last_unit : 6;
		int8_t turn : 8;
		Constants::CombatResult result : 3;
		int8_t order_size : 6;
		UnitId* order{ nullptr };

		// attacker state
		HeroStats attacker_stats{ 0 };
		uint8_t attacker_units_size : 3;
		CombatUnitPacked* attacker_units{ nullptr };

		// defender state
		HeroStats defender_stats{ 0 };
		uint8_t defender_units_size : 3;
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