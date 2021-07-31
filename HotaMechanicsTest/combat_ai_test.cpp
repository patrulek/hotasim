#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"
std::unordered_map<std::string, Unit> unit_templates{
	/* Simplest units: only walking, no abilities, only melee, etc */
		{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
		{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
		{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
		{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 3, 4, 0}, SpellBook{}, "Imp" } }
};

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "utils.h"


CombatAI& getAI() {
	static CombatHero attacker;
	static CombatHero defender;
	static CombatField field;

	static CombatManager combat_manager(attacker, defender, field);
	static auto& ai = combat_manager.getCombatAI();
	return const_cast<CombatAI&>(ai);
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

bool isAttackOrSpellcastAction(CombatAction action) {
	return action.action == CombatActionType::ATTACK || action.action == CombatActionType::SPELLCAST;
}

TEST(CombatAI, shouldReturnNoAttackNorSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndNoHostileUnitsInRange) {
	auto unit = CombatUnit(unit_templates["Peasant"]);

	CombatHero hero;
	hero.units[0] = unit;
	unit.hero = &hero;

	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	unit.hexId = getHexId(8, 1);

	auto& ai = getAI();

	auto actions = ai.generateActionsForPlayer(unit);
	EXPECT_EQ(22, actions.size()); // 20 walking actions, 1 wait action, 1 defend action

	int walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(20, walking_actions);

	/// --- ///

	auto& field = const_cast<CombatField&>(ai.combat_manager.getCombatField());
	field.hexes[8][2].occupiedBy = CombatHexOccupation::UNIT; // this unit prevents from going to 2 hexes, so there should be 20 actions now

	actions = ai.generateActionsForPlayer(unit);
	EXPECT_EQ(20, actions.size()); // 18 walking actions, 1 wait action, 1 defend action

	walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(18, walking_actions);

	/// --- ///

	unit.state.waiting = true; // were changing unit state, so now wait action should be gone
	actions = ai.generateActionsForPlayer(unit);
	EXPECT_EQ(19, actions.size()); // 18 walking actions, 1 defend action

	walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(18, walking_actions);
}

TEST(CombatAI, shouldReturnNoSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndHostileUnitsInRange) {
	CombatHero hero;
	auto unit = CombatUnit(unit_templates["Peasant"]);
	unit.hero = &hero;
	unit.applyHeroStats(hero.stats);
	unit.initUnit();
	unit.hexId = getHexId(8, 1);
	hero.units[0] = unit;


	CombatHero hero2;
	auto unit2 = CombatUnit(unit_templates["Peasant"]);
	unit2.hero = &hero2;
	unit2.applyHeroStats(hero2.stats);
	unit2.initUnit();
	unit2.hexId = getHexId(8, 2);
	hero2.units[0] = unit2;

	auto& ai = getAI();
	auto& field = const_cast<CombatField&>(ai.combat_manager.getCombatField());
	const_cast<CombatManager&>(ai.combat_manager).getCurrentState().heroes[0] = hero;
	const_cast<CombatManager&>(ai.combat_manager).getCurrentState().heroes[1] = hero2;
	field.hexes[8][2].occupiedBy = CombatHexOccupation::UNIT;

	auto actions = ai.generateActionsForPlayer(unit);
	EXPECT_EQ(26, actions.size()); // 18 walking actions, 6 attack actions, 1 wait action, 1 defend action

	int walking_actions = 0;
	int attack_actions = 0;
	for (auto action : actions) {
		walking_actions += (action.action == CombatActionType::WALK);
		attack_actions += (action.action == CombatActionType::ATTACK);
	}
	EXPECT_EQ(18, walking_actions);
	EXPECT_EQ(6, attack_actions);
}