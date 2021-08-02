#pragma once

#include <string>
#include <array>
#include <vector>

using int8 = char;
using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

struct PrimaryStats {
	uint16 atk;
	uint16 def;
	uint16 min_dmg;
	uint16 max_dmg;
	uint16 pow;
	uint16 kdg;
	uint16 spd;
	uint16 hp;
	uint16 mana;
	uint16 shots;
};

enum class HeroSkills {

};

enum class UnitSkills {

};

enum class SpellArea {
	ENEMY_UNIT, ENEMY_HERO, FRIENDLY_UNIT, FRIENDLY_HERO, ALL, SINGLE_HEX // etc
};

enum class SpellEffect {
	MOD_SPEED, MOD_ATTACK, MOD_DEFENCE, DAMAGE // etc
};

enum class Spells {
	/* 1st level */
	/* 2nd level */
	/* 3rd level */
	/* 4th level */
	/* 5th level */
	/* unit spells */
};

struct SpellBook {
	std::vector<Spells> spells;
};

struct Equipment {

};

enum class SpellID {
	SLOW,
	HASTE
};


struct UnitState {
	bool is_alive;
	bool is_summon;
	bool is_clone;
	bool morale;
	bool waiting;
	bool done;
	bool defending;
	bool sacrificed;
	bool retaliated;
};

struct Unit {
	uint16 fightValue;
	PrimaryStats stats;
	SpellBook spellbook;
	std::string name;

	//Unit(const Unit& unit) : fightValue(unit.fightValue), flags(unit.flags), stats(unit.stats), spells(unit.spells) {}
};


enum class CombatResult {
	NOT_STARTED, IN_PROGRESS, DRAW, PLAYER, ENEMY
};