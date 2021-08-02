#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"
std::unordered_map<std::string, Unit> unit_templates2{
	/* Simplest units: only walking, no abilities, only melee, etc */
		{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
		{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
		{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
		{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
};

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "utils.h"

TEST(CombatAI, shouldReturnCorrectMeleeUnitDmgWithoutAnyModifiers) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 200;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	// 200 imps should do 315 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
	EXPECT_EQ(315, mgr.calculateMeleeUnitAverageDamage(unit, unit2));

	// 500 peasants should do 475 dmg on average to 200 imps without any attack/defense and/or spells modifiers
	EXPECT_EQ(475, mgr.calculateMeleeUnitAverageDamage(unit2, unit));

	unit.stackNumber = 100;
	// 100 imps should do 157 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
	EXPECT_EQ(157, mgr.calculateMeleeUnitAverageDamage(unit, unit2));

	// 500 peasants should do 400 dmg on average to 100 imps (killing all) without any attack/defense and/or spells modifiers
	EXPECT_EQ(400, mgr.calculateMeleeUnitAverageDamage(unit2, unit));

	unit.stackNumber = 50;
	// 50 imps should do 78 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
	EXPECT_EQ(78, mgr.calculateMeleeUnitAverageDamage(unit, unit2));

	// 500 peasants should do 200 dmg on average to 50 imps (killing all) without any attack/defense and/or spells modifiers
	EXPECT_EQ(200, mgr.calculateMeleeUnitAverageDamage(unit2, unit));

	unit = CombatUnit(unit_templates2["Goblin"]);
	unit.hero = &hero;
	unit.stackNumber = 30;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	// 30 goblins should do 51 dmg on average to peasants without any attack/defense and/or spells modifiers
	EXPECT_EQ(51, mgr.calculateMeleeUnitAverageDamage(unit, unit2));
}


TEST(CombatAI, shouldReturnZeroForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsKilledOrCantRetaliate) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 200;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 200;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	// 200 imps should kill peasants, so 0 dmg in counterattack
	EXPECT_EQ(0, mgr.calculateCounterAttackMeleeUnitAverageDamage(unit, unit2));


	unit2.stackNumber = 500;
	unit2.initUnit();
	unit2.state.retaliated = true;
	// 200 imps will not kill all peasants, but they already retaliated so 0 dmg
	EXPECT_EQ(0, mgr.calculateCounterAttackMeleeUnitAverageDamage(unit, unit2));
}


TEST(CombatAI, shouldReturnCorrectValueForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsNotKilledAndCanRetaliate) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 200;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	// 200 imps should kill 315 peasants, so 185 peasants should retaliate 175 dmg to imps
	EXPECT_EQ(175, mgr.calculateCounterAttackMeleeUnitAverageDamage(unit, unit2));

	// 500 peasants should kill 118 imps, so 82 imps should retaliate 129 to peasants
	EXPECT_EQ(129, mgr.calculateCounterAttackMeleeUnitAverageDamage(unit2, unit));
}



TEST(CombatAI, shouldReturnCorrectFightValueGainForMeleeUnitAttackWithoutAnyModifiersWhenSimilarArmyStrength) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 200;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	// fight value gain when 200 imps attack 500 peasants first
	EXPECT_EQ(2538, mgr.calculateFightValueAdvantageAfterMeleeUnitAttack(unit, unit2));

	// fight value gain when 500 peasants attack 200 imps first
	EXPECT_EQ(4002, mgr.calculateFightValueAdvantageAfterMeleeUnitAttack(unit2, unit));
}

TEST(CombatAI, shouldReturnOnlyZeroFightValueGainOnHexesForMeleeUnitWhenAttackerWeakerThanDefender) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 50;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	std::vector<int> expected(187, 0);
	EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(unit2, hero));
}

TEST(CombatAI, shouldReturnCorrectFightValueGainOnHexesForMeleeUnitWhenAttackerLittleStrongerThanDefender) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 200;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	unit.hexId = getHexId(5, 1);
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.hexId = getHexId(5, 15);
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	int fv = -2538;
	std::vector<int> expected{
		0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(unit2, hero));
}


TEST(CombatAI, DISABLED_shouldReturnMaximumFightValueGainOnHexesForMeleeUnitWhenAttackerIncrediblyStrongerThanDefender) {
	CombatHero hero; // 0 atk, 0 def
	auto unit = CombatUnit(unit_templates2["Imp"]);
	unit.hero = &hero;
	unit.stackNumber = 301;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	unit.hexId = getHexId(5, 1);
	hero.units[0] = unit;

	CombatHero hero2; // 0 atk, 0 def
	auto unit2 = CombatUnit(unit_templates2["Peasant"]);
	unit2.hero = &hero2;
	unit2.stackNumber = 500;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.hexId = getHexId(5, 15);
	unit2.state.retaliated = false;
	hero2.units[0] = unit2;

	CombatField field;
	CombatManager mgr(hero, hero2, field);

	int fv = -7500; // TODO: implement case when attacker fight value > 2 * defender fight value; check if there isnt case for 5 * also
	std::vector<int> expected{
		0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(unit2, hero));
}