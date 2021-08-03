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



struct UnitPermutation {
	int unit_id;
	int unit_order;
	int unit_number;

	bool operator==(const UnitPermutation& _obj) const {
		return unit_id == _obj.unit_id && unit_order == _obj.unit_order && unit_number == _obj.unit_number;
	}
};


struct CombatFieldSize {
	static const int COLS = 17;
	static const int ROWS = 11;
};

enum class CombatSide {
	ATTACKER, DEFENDER
};

struct ArmyPermutation
{
	std::vector<UnitPermutation> permutations;

	bool operator==(const ArmyPermutation& _obj) const {
		return permutations == _obj.permutations;
	}
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

enum class CombatType {
	NEUTRAL, ENCOUNTER
};

struct UnitStack {
	Unit unit;
	int stack_number;
};

struct Hero {
	PrimaryStats stats{};
	HeroSkills skills{};
	SpellBook spells{};
	Equipment artifacts{};
	std::vector<UnitStack> army{};

	~Hero() {}

	void setAttack(const int _attack) { stats.atk = _attack; }
	void setDefense(const int _defense) { stats.def = _defense; }
	void setPower(const int _power) { stats.pow = _power; }
	void setKnowledge(const int _knowledge) { stats.kdg = _knowledge; }

	void setHeroSkills(const HeroSkills& _skills) { skills = _skills; }
	void addHeroSkill() { throw std::exception("Not implemented yet"); }
	void removeHeroSkill() { throw std::exception("Not implemented yet"); }

	void setSpellBook(const SpellBook& _spellbook) { spells = _spellbook; }
	void addSpell() { throw std::exception("Not implemented yet"); }
	void removeSpell() { throw std::exception("Not implemented yet"); }

	void setEquipment(const Equipment& _equipment) { artifacts = _equipment; }
	void addArtifact() { throw std::exception("Not implemented yet"); }
	void removeArtifact() { throw std::exception("Not implemented yet"); }

	void setHeroArmy(const std::vector<UnitStack>& _army) {
		army = _army;
	}

	void addUnit(const Unit& _unit, const int _stack_size) {
		addUnitStack(UnitStack{ _unit, _stack_size });
	}

	void addUnitStack(const UnitStack& _unit_stack) {
		if (army.size() >= 7)
			return;
		army.push_back(_unit_stack);
	}

	void removeUnitStack(const UnitStack&) { throw std::exception("Not implemented yet"); }

	void removeHeroArmy() { army.clear(); }

	ArmyPermutation generateBaseArmyPermutation() const {
		ArmyPermutation permutation;

		for (int i = 0; i < army.size(); ++i)
			permutation.permutations.push_back(UnitPermutation{ i, i, army[i].stack_number });

		return permutation;
	}
};