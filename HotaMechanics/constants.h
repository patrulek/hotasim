#pragma once

#include <vector>
#include <cstdint>


namespace HotaMechanics {
	using HexId = uint8_t;
	using HexAreaId = uint8_t;
	using UnitId = uint8_t;
	using Hash = int64_t;
}

namespace HotaMechanics::Constants {
	static const int16_t MANA_PER_KGD = 10;
	static const uint8_t FIELD_COLS = 17;
	static const uint8_t FIELD_ROWS = 11;
	static constexpr uint8_t FIELD_SIZE = FIELD_COLS * FIELD_ROWS;
	static const HexId INVALID_HEX_ID = FIELD_SIZE;
	static const UnitId GUID_OFFSET = 21;
	static const UnitId INVALID_UNIT_ID = 255;
	static const uint8_t BASE_ARMY_SIZE = 7;
	static const uint8_t MAX_FIELD_RANGE = 254;


	using AdjacentArray = std::array<HexId, 6>;
	using FieldArray = std::array<HexId, FIELD_SIZE + 1>;
	using FieldFlagArray = std::array<bool, FIELD_SIZE + 1>;
	static std::vector<HexId> EMPTY_PATH;
	static std::vector<HexId> EMPTY_VEC;

	enum class AIDifficulty : int8_t {
		EASY /*80%, 100%*/, NORMAL /*130%, 160%, 200%*/
	};

	enum class CombatSide : bool {
		ATTACKER, DEFENDER
	};

	enum class HeroSpecs : int8_t {

	};

	enum class HeroSkills : int8_t {

	};

	enum class UnitSkills : int8_t {

	};

	enum class SpellArea : int8_t {
		ENEMY_UNIT, ENEMY_HERO, FRIENDLY_UNIT, FRIENDLY_HERO, ALL, SINGLE_HEX // etc
	};

	enum class SpellEffect : int8_t {
		MOD_SPEED, MOD_ATTACK, MOD_DEFENCE, DAMAGE // etc
	};

	enum class Spells : int8_t {
		/* 1st level */
		SLOW, HASTE // etc
		/* 2nd level */
		/* 3rd level */
		/* 4th level */
		/* 5th level */
		/* unit spells */
	};

	enum class CombatHexOccupation : int8_t {
		EMPTY, UNIT, SOFT_OBSTACLE, SOLID_OBSTACLE, INDESTRUCTIBLE_OBSTACLE // etc
	};

	enum class CombatFieldType : int8_t {
		GRASS, DIRT // ...
	};

	enum class CombatFieldTemplate : int8_t {
		EMPTY, TMP1, TMP2, IMPS_2x100 // ...
	};

	enum class CombatActionType : int8_t {
		WALK, ATTACK, WAIT, DEFENSE, SPELLCAST, PRE_BATTLE, PRE_TURN
	};

	enum class CombatResult : int8_t {
		NOT_STARTED, IN_PROGRESS, DRAW, PLAYER, ENEMY
	};

	enum class CombatType : int8_t {
		NEUTRAL, ENCOUNTER
	};


	enum class CombatEventType : int8_t {
		UNIT_POS_CHANGED, FIELD_CHANGE, UNIT_HEALTH_LOST, UNIT_STATS_CHANGED, BATTLE_START
	};



} // HotaMechanics::Constants