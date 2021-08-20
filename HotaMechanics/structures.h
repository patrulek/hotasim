#pragma once

#include <string>
#include <array>
#include <vector>
#include <list>

#include <cstdint>

#include "constants.h"

namespace HotaMechanics {
	union BaseStats {
		int stats;
		struct {
			int8_t atk;
			int8_t def;
			int8_t pow;
			int8_t kgd;
		};
	};

	union CombatStats {
		int stats;
		struct {
			uint8_t min_dmg;
			uint8_t max_dmg;
			int8_t spd;
			int8_t shots;
		};
	};

	union PrimaryStats {
		int stats;
		struct {
			int16_t hp;
			int16_t mana;
		};
	};

	struct UnitStats {
		BaseStats base_stats{ 0 };
		CombatStats combat_stats{ 0 };
		PrimaryStats primary_stats{ 0 };
		uint16_t fight_value{ 0 };
	};

	struct HeroStats {
		BaseStats base_stats{ 0 };
		PrimaryStats primary_stats{ 0 };
	};

	struct UnitPermutation {
		UnitId unit_id{ Constants::INVALID_UNIT_ID };
		uint8_t unit_order{ 0 };
		int16_t unit_number{ 0 };

		bool operator==(const UnitPermutation& _obj) const {
			return unit_id == _obj.unit_id && unit_order == _obj.unit_order && unit_number == _obj.unit_number;
		}
	};

	struct ArmyPermutation
	{
		std::vector<UnitPermutation> permutations;

		bool operator==(const ArmyPermutation& _obj) const {
			return permutations == _obj.permutations;
		}
	};

	struct SpellBook {
		std::vector<Constants::Spells> spells{};
	};

	struct Equipment {

	};

	struct Unit {
		UnitStats stats{ 0 };
		std::string name{ "undefined" };
	};

	struct UnitStack {

		const Unit* unit{ nullptr };
		int16_t stack_number{ 0 };
	};

	struct Hero {
		HeroStats stats{ 0 };
		std::vector<UnitStack> army{};
	};

	struct CombatAction {
		Constants::CombatActionType action{ Constants::CombatActionType::DEFENSE };
		int16_t param1{ -1 }; // unit_id (target for attack/spellcast) : walk_distance (for walk action)
		HexId target{ Constants::INVALID_HEX_ID }; // hex_id (for walk/melee attack/spellcast)
		int16_t param2{ 0 }; // true/false (whether action ends unit turn; hero spellcast dont)
	};

	struct CombatEvent {
		Constants::CombatEventType type{ Constants::CombatEventType::BATTLE_START };
		int16_t param1{ -1 };
		HexId param2{ Constants::INVALID_HEX_ID }; // source hex
		HexId param3{ Constants::INVALID_HEX_ID }; // target hex
	};
}; // HotaMechanics