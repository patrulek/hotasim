
#include "gtest/gtest.h"


#include <unordered_map>
#include "../HotaMechanics/structures.h"
std::unordered_map<std::string, Unit> unit_templates_1{
	/* Simplest units: only walking, no abilities, only melee, etc */
		{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
		{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
		{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
		{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
};

#include "../HotaMechanics/combat_unit.h"

TEST(CombatUnit, shouldReturnStackNumberTimesCurrentHpWhenNoHealthlost) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.stackNumber = 100;
	EXPECT_EQ(100, unit.calculateStackHP());

	unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 0;
	EXPECT_EQ(0, unit.calculateStackHP());

	unit = CombatUnit(unit_templates_1["Goblin"]);
	unit.stackNumber = 11;
	EXPECT_EQ(55, unit.calculateStackHP());
}

TEST(CombatUnit, shouldReturnStackNumberTimesCurrentHpMinutHealthlost) {
	auto unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 1; unit.health_lost = 2;
	EXPECT_EQ(2, unit.calculateStackHP());

	unit = CombatUnit(unit_templates_1["Goblin"]);
	unit.stackNumber = 11; unit.health_lost = 1;
	EXPECT_EQ(54, unit.calculateStackHP());
}

TEST(CombatUnit, shouldReturnSingleUnitFightValueWithoutModifiers) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.stackNumber = 100;
	EXPECT_EQ(15, unit.getSingleUnitFightValue());

	unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 50; unit.health_lost = 2;
	EXPECT_EQ(50, unit.getSingleUnitFightValue());
}

TEST(CombatUnit, shouldReturnStackUnitFightValueWithoutModifiers) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.stackNumber = 100;
	EXPECT_EQ(1500, unit.getStackUnitFightValue());

	unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 50; unit.health_lost = 1;
	EXPECT_EQ(2487.5, unit.getStackUnitFightValue());

	unit.health_lost = 2;
	EXPECT_EQ(2475, unit.getStackUnitFightValue());

	unit.health_lost = 3;
	EXPECT_EQ(2462.5, unit.getStackUnitFightValue());
}

TEST(CombatUnit, shouldApplyHeroStatsOnlyOnce) {
	PrimaryStats hero_stats;
	hero_stats.atk = 2; hero_stats.def = 1;

	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.applyHeroStats(hero_stats);

	EXPECT_EQ(3, unit.currentStats.atk);
	EXPECT_EQ(2, unit.currentStats.def);

	unit.applyHeroStats(hero_stats);

	EXPECT_EQ(3, unit.currentStats.atk);
	EXPECT_EQ(2, unit.currentStats.def);
}

TEST(CombatUnit, shouldReturnAttackDiffWithoutModifiers) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);

	EXPECT_EQ(0, unit.calcDiffAtk());
	EXPECT_EQ(0, unit.calcDiffDef());

	PrimaryStats hero_stats;
	hero_stats.atk = 2; hero_stats.def = 1;
	unit.applyHeroStats(hero_stats);

	EXPECT_EQ(2, unit.calcDiffAtk());
	EXPECT_EQ(1, unit.calcDiffDef());
}

TEST(CombatUnit, shouldReturnBaseAverageDmgForMeleeUnit) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	EXPECT_FLOAT_EQ(1.0f, unit.getBaseAverageDmg());

	unit = CombatUnit(unit_templates_1["Imp"]);
	EXPECT_FLOAT_EQ(1.5f, unit.getBaseAverageDmg());
}

TEST(CombatUnit, shouldKillStackIfDmgGreaterEqualStackHp) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.stackNumber = 500;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(500);
	EXPECT_FALSE(unit.isAlive());
	EXPECT_EQ(0, unit.stackNumber);
	EXPECT_EQ(0, unit.health_lost);

	unit.stackNumber = 500;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(501);
	EXPECT_FALSE(unit.isAlive());
	EXPECT_EQ(0, unit.stackNumber);
	EXPECT_EQ(0, unit.health_lost);

	unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 100;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(501);
	EXPECT_FALSE(unit.isAlive());
	EXPECT_EQ(0, unit.stackNumber);
	EXPECT_EQ(0, unit.health_lost);
}


TEST(CombatUnit, shouldSpareSomeUnitsIfDmgLesserThanStackHP) {
	auto unit = CombatUnit(unit_templates_1["Peasant"]);
	unit.stackNumber = 500;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(300);
	EXPECT_TRUE(unit.isAlive());
	EXPECT_EQ(200, unit.stackNumber);
	EXPECT_EQ(0, unit.health_lost);

	unit = CombatUnit(unit_templates_1["Imp"]);
	unit.stackNumber = 100;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(20);
	EXPECT_TRUE(unit.isAlive());
	EXPECT_EQ(95, unit.stackNumber);
	EXPECT_EQ(0, unit.health_lost);

	unit.stackNumber = 100;
	unit.initUnit();

	EXPECT_TRUE(unit.isAlive());
	unit.applyDamage(23);
	EXPECT_TRUE(unit.isAlive());
	EXPECT_EQ(95, unit.stackNumber);
	EXPECT_EQ(3, unit.health_lost);
}