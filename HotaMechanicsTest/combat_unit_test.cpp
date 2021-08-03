
#include "gtest/gtest.h"


#include <unordered_map>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_unit.h"
#include "../HotaMechanics/combat_hero.h"

namespace CombatUnitTest {
	std::unordered_map<std::string, Unit> unit_templates{
		/* Simplest units: only walking, no abilities, only melee, etc */
			{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
			{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
			{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
			{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
	};

	CombatHero createHero(std::vector<UnitStack>& _army, int _atk = 0, int _def = 0) {
		Hero tmp;
		tmp.setAttack(_atk); tmp.setDefense(_def);
		tmp.setHeroArmy(_army);

		return CombatHero{ tmp };
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

	TEST(CombatUnit, shouldReturnStackNumberTimesCurrentHpWhenNoHealthlost) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(100, unit->calculateStackHP());

		hero = createHero(createArmy("Imp", 0));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(0, unit->calculateStackHP());

		hero = createHero(createArmy("Goblin", 11));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(55, unit->calculateStackHP());
	}

	TEST(CombatUnit, shouldReturnStackNumberTimesCurrentHpMinusHealthlost) {
		auto hero = createHero(createArmy("Imp", 1));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->health_lost = 2;
		EXPECT_EQ(2, unit->calculateStackHP());

		hero = createHero(createArmy("Goblin", 11));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->health_lost = 1;
		EXPECT_EQ(54, unit->calculateStackHP());
	}

	TEST(CombatUnit, shouldReturnSingleUnitFightValueWithoutModifiers) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(15, unit->getSingleUnitFightValue());

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->health_lost = 2;
		EXPECT_EQ(50, unit->getSingleUnitFightValue());
	}

	TEST(CombatUnit, shouldReturnStackUnitFightValueWithoutModifiers) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(1500, unit->getStackUnitFightValue());

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->health_lost = 1;
		EXPECT_EQ(2487.5, unit->getStackUnitFightValue());

		unit->health_lost = 2;
		EXPECT_EQ(2475, unit->getStackUnitFightValue());

		unit->health_lost = 3;
		EXPECT_EQ(2462.5, unit->getStackUnitFightValue());
	}

	TEST(CombatUnit, shouldApplyHeroStatsOnlyOnce) {
		auto hero = createHero(createArmy("Peasant", 100), 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();

		EXPECT_EQ(3, unit->currentStats.atk);
		EXPECT_EQ(2, unit->currentStats.def);

		unit->applyHeroStats();

		EXPECT_EQ(3, unit->currentStats.atk);
		EXPECT_EQ(2, unit->currentStats.def);
	}

	TEST(CombatUnit, shouldReturnAttackDiffWithoutModifiers) {
		auto hero = createHero(createArmy("Peasant", 100), 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		EXPECT_EQ(0, unit->calcDiffAtk());
		EXPECT_EQ(0, unit->calcDiffDef());

		unit->applyHeroStats();

		EXPECT_EQ(2, unit->calcDiffAtk());
		EXPECT_EQ(1, unit->calcDiffDef());
	}

	TEST(CombatUnit, shouldReturnBaseAverageDmgForMeleeUnit) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_FLOAT_EQ(1.0f, unit->getBaseAverageDmg());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_FLOAT_EQ(1.5f, unit->getBaseAverageDmg());
	}

	TEST(CombatUnit, shouldKillStackIfDmgGreaterEqualStackHp) {

		auto hero = createHero(createArmy("Peasant", 500));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(500);
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->stackNumber);
		EXPECT_EQ(0, unit->health_lost);

		hero = createHero(createArmy("Peasant", 500));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501);
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->stackNumber);
		EXPECT_EQ(0, unit->health_lost);

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501);
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->stackNumber);
		EXPECT_EQ(0, unit->health_lost);
	}


	TEST(CombatUnit, shouldSpareSomeUnitsIfDmgLesserThanStackHP) {
		auto hero = createHero(createArmy("Peasant", 500));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(300);
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(200, unit->stackNumber);
		EXPECT_EQ(0, unit->health_lost);

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(20);
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->stackNumber);
		EXPECT_EQ(0, unit->health_lost);
		
		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(23);
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->stackNumber);
		EXPECT_EQ(3, unit->health_lost);
	}
}