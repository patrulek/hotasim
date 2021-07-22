#pragma once

using int8 = char;
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

struct PrimaryStats {
	uint16 atk;
	uint16 def;
	uint16 pow;
	uint16 kdg;
	uint16 spd;
	uint16 hp;
	uint16 mana;
};

struct SecondarySkills {

};

struct SpellBook {

};

struct Equipment {

};

struct Unit {
	uint16 fightValue;
	uint32 flags;
	PrimaryStats stats;
	SpellBook spells;

	//Unit(const Unit& unit) : fightValue(unit.fightValue), flags(unit.flags), stats(unit.stats), spells(unit.spells) {}
};


enum class CombatResult {
	NOT_STARTED, IN_PROGRESS, DRAW, PLAYER, ENEMY
};

enum class CombatActionType {
	UNDEFINED, MOVE, DEFENSE, CALCULATE, UNKNOWN_4, MELEE, SHOOT, WAIT, UNKNOWN_8, UNKOWN_9, REGEN, UNKNOWN_11
};

struct CombatAction {
	CombatActionType type{ CombatActionType::UNDEFINED };
};
