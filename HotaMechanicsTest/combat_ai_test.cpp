#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"
std::unordered_map<std::string, Unit> unit_templates{
	/* Simplest units: only walking, no abilities, only melee, etc */
		{"Peasant", { 15, 16, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
		{ "Gremlin", { 55, 16, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
		{ "Goblin", { 60, 16, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
		{ "Imp", { 50, 16, {2, 3, 1, 2, 0, 0, 3, 4, 0}, SpellBook{}, "Imp" } }
};

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"

const CombatAI& getAI() {
	static CombatHero attacker;
	static CombatHero defender;
	static CombatField field;

	static CombatManager combat_manager(attacker, defender, field);
	static const auto& ai = combat_manager.getCombatAI();
	return ai;
}

TEST(CombatAI, shouldReturn1IfNoOtherModifiersAndEqualStats) {
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.stackNumber = 100;
	
	auto& ai = getAI();

	EXPECT_EQ(1.0f, ai.calculateUnitAttackFightValueModifier(unit));
	EXPECT_EQ(1.0f, ai.calculateUnitDefenceFightValueModifier(unit));
}


TEST(CombatAI, shouldReturnGreaterThan1IfNoOtherModifiersAndGreaterStats) {
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.stackNumber = 100;
	PrimaryStats hero_stats; 
	hero_stats.atk = 2; hero_stats.def = 1;
	unit.applyHeroStats(hero_stats);

	auto& ai = getAI();

	EXPECT_EQ(1.1f, ai.calculateUnitAttackFightValueModifier(unit));
	EXPECT_EQ(1.05f, ai.calculateUnitDefenceFightValueModifier(unit));
}

TEST(CombatAI, shouldReturnSqrtOfAtkModiferTimesDefModifier) {
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.stackNumber = 100;
	PrimaryStats hero_stats;
	hero_stats.atk = 2; hero_stats.def = 1;

	auto& ai = getAI();

	EXPECT_EQ(1.0f, ai.calculateUnitFightValueModifier(unit));

	unit.applyHeroStats(hero_stats);

	EXPECT_FLOAT_EQ(1.074709f, ai.calculateUnitFightValueModifier(unit));
}

TEST(CombatAI, shouldReturnUnitFightValueModifierTimesStackUnitFightValue) {
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.stackNumber = 100;
	PrimaryStats hero_stats;
	hero_stats.atk = 2; hero_stats.def = 1;

	auto& ai = getAI();

	EXPECT_EQ(1500, ai.calculateStackUnitFightValue(unit));

	unit.applyHeroStats(hero_stats);

	EXPECT_EQ(1612, ai.calculateStackUnitFightValue(unit));
}

TEST(CombatAI, shouldReturnSummedUnitFightValuesForHero) {
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.stackNumber = 100;
	PrimaryStats hero_stats;
	hero_stats.atk = 2; hero_stats.def = 1;

	CombatHero hero;
	hero.units[0] = unit;

	auto& ai = getAI();

	EXPECT_EQ(0, ai.calculateHeroFightValue(hero));

	unit.applyHeroStats(hero_stats);
	hero.units[0] = unit;

	EXPECT_EQ(1612, ai.calculateHeroFightValue(hero));

	unit.stackNumber = 49;
	hero.units[1] = unit;

	EXPECT_EQ(1612 + 789, ai.calculateHeroFightValue(hero));
}
