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
		BaseStats base_stats;
		CombatStats combat_stats;
		PrimaryStats primary_stats;
		uint16_t fight_value;
	};

	struct HeroStats {
		BaseStats base_stats;
		PrimaryStats primary_stats;
	};

	struct UnitPermutation {
		int8_t unit_id;
		int8_t unit_order;
		int16_t unit_number;

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
		std::vector<Constants::Spells> spells;
	};

	struct Equipment {

	};

	struct Unit {
		UnitStats stats;
		std::string name;
	};

	struct UnitStack {
		Unit unit;
		int16_t stack_number;
	};

	struct Hero {
		HeroStats stats;
		std::vector<UnitStack> army;

		~Hero() {}
	};

	struct CombatAction {
		Constants::CombatActionType action;
		int16_t param1; // unit_id (target for attack/spellcast) : walk_distance (for walk action)
		int16_t target; // hex_id (for walk/melee attack/spellcast)
		int16_t param2; // true/false (whether action ends unit turn; hero spellcast dont)
	};
}; // HotaMechanics