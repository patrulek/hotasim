#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"
std::unordered_map<std::string, Unit> unit_templates{
	/* Simplest units: only walking, no abilities, only melee, etc */
		{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
		{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
		{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
		{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
};

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "utils.h"



std::vector<int> getCombatFieldTemplate(const int type) {
	// TODO: generate different field templates
	if (type == 1)
		return std::vector<int>{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};


	return std::vector<int>{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
}

CombatAI& getAI() {
	static CombatHero attacker;
	static CombatHero defender;
	static CombatField field;

	static CombatManager combat_manager(attacker, defender, field, CombatType::NEUTRAL);
	static auto& ai = combat_manager.getCombatAI();
	return const_cast<CombatAI&>(ai);
}


bool isAttackOrSpellcastAction(CombatAction action) {
	return action.action == CombatActionType::ATTACK || action.action == CombatActionType::SPELLCAST;
}

std::vector<UnitStack> createArmy(const std::string tmp1, const int size1,
											const std::string tmp2 = "", const int size2 = 0,
											const std::string tmp3 = "", const int size3 = 0,
											const std::string tmp4 = "", const int size4 = 0,
											const std::string tmp5 = "", const int size5 = 0,
											const std::string tmp6 = "", const int size6 = 0,
											const std::string tmp7 = "", const int size7 = 0) {

	std::vector<UnitStack> army{ UnitStack{ unit_templates[tmp1], size1 } };
	if (size2 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp2], size2 });
	if (size3 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp3], size3 });
	if (size4 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp4], size4 });
	if (size5 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp5], size5 });
	if (size6 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp6], size6 });
	if (size7 == 0) return army;

	army.emplace_back(UnitStack{ unit_templates[tmp7], size7 });
	return army;
}

CombatHero createHero(std::vector<UnitStack>& _army, int _atk = 0, int _def = 0) {
	Hero tmp;
	tmp.setAttack(_atk); tmp.setDefense(_def);
	tmp.setHeroArmy(_army);

	return CombatHero{ tmp };
}

TEST(CombatAI, shouldReturn1IfNoOtherModifiersAndEqualStats) {
	auto hero = createHero(createArmy("Peasant", 100));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	
	auto& ai = getAI();

	EXPECT_EQ(1.0f, ai.calculateUnitAttackFightValueModifier(*unit));
	EXPECT_EQ(1.0f, ai.calculateUnitDefenceFightValueModifier(*unit));
}


TEST(CombatAI, shouldReturnGreaterThan1IfNoOtherModifiersAndGreaterStats) {
	auto hero = createHero(createArmy("Peasant", 100), 2, 1);
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	unit->applyHeroStats();
	auto& ai = getAI();

	EXPECT_EQ(1.1f, ai.calculateUnitAttackFightValueModifier(*unit));
	EXPECT_EQ(1.05f, ai.calculateUnitDefenceFightValueModifier(*unit));
}

TEST(CombatAI, shouldReturnSqrtOfAtkModiferTimesDefModifier) {
	auto hero = createHero(createArmy("Peasant", 100), 2, 1);
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	auto& ai = getAI();

	EXPECT_EQ(1.0f, ai.calculateUnitFightValueModifier(*unit));

	unit->applyHeroStats();

	EXPECT_FLOAT_EQ(1.074709f, ai.calculateUnitFightValueModifier(*unit));
}

TEST(CombatAI, shouldReturnUnitFightValueModifierTimesStackUnitFightValue) {
	auto hero = createHero(createArmy("Peasant", 100), 2, 1);
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	auto& ai = getAI();

	EXPECT_EQ(1500, ai.calculateStackUnitFightValue(*unit));

	unit->applyHeroStats();

	EXPECT_EQ(1612, ai.calculateStackUnitFightValue(*unit));
}

TEST(CombatAI, shouldReturnSummedUnitFightValuesForHero) {
	auto hero = createHero(createArmy("Peasant", 100), 2, 1);
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	auto& ai = getAI();

	EXPECT_EQ(1500, ai.calculateHeroFightValue(hero));

	unit->applyHeroStats();

	EXPECT_EQ(1612, ai.calculateHeroFightValue(hero));

	hero = createHero(createArmy("Peasant", 100, "Peasant", 49), 2, 1);
	for (auto unit : hero.getUnits())
		const_cast<CombatUnit*>(unit)->applyHeroStats();

	EXPECT_EQ(1612 + 789, ai.calculateHeroFightValue(hero));
}

TEST(CombatAI, shouldReturnNoAttackNorSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndNoHostileUnitsInRange) {
	auto hero = createHero(createArmy("Peasant", 100));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	auto& ai = getAI();

	unit->applyHeroStats();
	unit->initUnit();
	unit->moveTo(getHexId(8, 1));

	auto actions = ai.generateActionsForPlayer(*unit);
	EXPECT_EQ(22, actions.size()); // 20 walking actions, 1 wait action, 1 defend action

	int walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(20, walking_actions);

	/// --- ///

	auto& field = const_cast<CombatField&>(ai.combat_manager.getCombatField());
	field.fillHex(getHexId(8, 2), CombatHexOccupation::SOLID_OBSTACLE); // this obstacle prevents from going to 2 hexes, so there should be 20 actions now

	actions = ai.generateActionsForPlayer(*unit);
	EXPECT_EQ(20, actions.size()); // 18 walking actions, 1 wait action, 1 defend action

	walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(18, walking_actions);

	/// --- ///

	unit->state.waiting = true; // were changing unit state, so now wait action should be gone
	actions = ai.generateActionsForPlayer(*unit);
	EXPECT_EQ(19, actions.size()); // 18 walking actions, 1 defend action

	walking_actions = 0;
	for (auto action : actions) {
		EXPECT_FALSE(isAttackOrSpellcastAction(action));
		walking_actions += (action.action == CombatActionType::WALK);
	}
	EXPECT_EQ(18, walking_actions);
}

TEST(CombatAI, shouldReturnNoSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndHostileUnitsInRange) {
	auto hero = createHero(createArmy("Peasant", 100));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	unit->applyHeroStats();
	unit->initUnit();
	unit->moveTo(getHexId(8, 1));

	auto hero2 = createHero(createArmy("Peasant", 100));
	auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
	unit2->applyHeroStats();
	unit2->initUnit();
	unit2->moveTo(getHexId(8, 2));

	auto& ai = getAI();
	auto& field = const_cast<CombatField&>(ai.combat_manager.getCombatField());
	const_cast<CombatManager&>(ai.combat_manager).getCurrentState().attacker = hero;
	const_cast<CombatManager&>(ai.combat_manager).getCurrentState().defender = hero2;
	field.fillHex(getHexId(8, 2), CombatHexOccupation::UNIT); // this unit prevents from going to 2 hexes

	auto actions = ai.generateActionsForPlayer(*unit);
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

TEST(CombatAI, shouldChooseExactlyThisUnitToAttackIfOnlyOneUnitStackLeftInEnemyHero) {
	auto hero = createHero(createArmy("Peasant", 500));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

	auto hero2 = createHero(createArmy("Imp", 200));
	auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
	unit2->applyHeroStats();
	unit2->initUnit();

	auto& ai = getAI();

	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(0, ai.chooseUnitToAttack(*unit, hero2).front());
}

TEST(CombatAI, shouldChooseSecondUnitStackToChaseWhenEqualStacksAndCannotAttackBoth) {
	auto hero = createHero(createArmy("Peasant", 500));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	unit->moveTo(getHexId(5, 15));

	auto hero2 = createHero(createArmy("Imp", 100, "Imp", 100));
	auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
	unit2->applyHeroStats();
	unit2->initUnit();
	unit2->moveTo(getHexId(2, 1));

	auto unit3 = const_cast<CombatUnit*>(hero2.getUnits().back());
	unit3->applyHeroStats();
	unit3->initUnit();
	unit3->moveTo(getHexId(8, 1));

	auto& ai = getAI();
	auto field = ai.combat_manager.getCombatField();
	field.setTemplate(getCombatFieldTemplate(1));

	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 13));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 10));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 7));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 4));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());
}


TEST(CombatAI, shouldChooseRandomlyFromTwoStacksIfEqualStacksAndCanAttackBoth) {
	auto hero = createHero(createArmy("Peasant", 500));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	unit->moveTo(getHexId(5, 15));

	auto hero2 = createHero(createArmy("Imp", 100, "Imp", 100));
	auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
	unit2->applyHeroStats();
	unit2->initUnit();
	unit2->moveTo(getHexId(2, 1));

	auto unit3 = const_cast<CombatUnit*>(hero2.getUnits().back());
	unit3->applyHeroStats();
	unit3->initUnit();
	unit3->moveTo(getHexId(8, 1));

	auto& ai = getAI();
	auto field = ai.combat_manager.getCombatField();
	field.setTemplate(getCombatFieldTemplate(1));

	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 13));
	unit2->moveTo(getHexId(7, 1));
	unit3->moveTo(getHexId(6, 1));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 10));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 7));
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).size());
	EXPECT_EQ(1, ai.chooseUnitToAttack(*unit, hero2).front());

	unit->moveTo(getHexId(7, 4));
	EXPECT_EQ(2, ai.chooseUnitToAttack(*unit, hero2).size());
}

TEST(CombatAI, shouldChooseCorrectHexWhichMeleeAttackWillBePerformedFrom) {
	auto hero = createHero(createArmy("Peasant", 500));
	auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
	unit->moveTo(getHexId(5, 15));
	unit->applyHeroStats();
	unit->initUnit();

	auto hero2 = createHero(createArmy("Imp", 200));
	auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
	unit2->moveTo(getHexId(5, 1));
	unit2->applyHeroStats();
	unit2->initUnit();

	auto& ai = getAI();
	
	auto field = ai.combat_manager.getCombatField();
	field.setTemplate(getCombatFieldTemplate(1));

	EXPECT_EQ(69, ai.chooseHexToMoveForAttack(*unit, *unit2));

	unit->moveTo(getHexId(4, 12));
	unit2->moveTo(getHexId(7, 5));
	EXPECT_EQ(107, ai.chooseHexToMoveForAttack(*unit, *unit2));

	unit->moveTo(getHexId(4, 10));
	unit2->moveTo(getHexId(10, 8));
	EXPECT_EQ(162, ai.chooseHexToMoveForAttack(*unit, *unit2));
}