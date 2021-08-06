#include "gtest/gtest.h"

#include "utils.h"

#include <unordered_map>
#include "../HotaMechanics/unit_templates.h"
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_unit.h"
#include "../HotaMechanics/combat_hero.h"

namespace CombatUnitTest {
	using namespace HotaMechanics;
	using namespace TestUtils;



	// CombatUnit::applyDamage(damage >= CombatUnit::getStackHP())
	TEST(CombatUnit, shouldKillStackIfDmgGreaterEqualStackHp) {
		auto hero = createHero(createArmy("Peasant", 500));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(500); // damage == CombatUnit::getStackHP()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		hero = createHero(createArmy("Peasant", 500));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501); // damage > CombatUnit::getStackHP()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501); // damage > CombatUnit::getStack()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());
	}

	// CombatUnit::applyDamage(damage >= 0 && damage < CombatUnit::getStackHP())
	TEST(CombatUnit, shouldSpareSomeUnitsIfDmgLesserThanStackHP) {
		auto hero = createHero(createArmy("Peasant", 500));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(300); // damage > 0 && damage < CombatUnit::getStackHP()
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(200, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(20); // damage == 5 * primary_stats.hp 
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(23); // damage > 0 && damage < CombatUnit::getStackHP()
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->getStackNumber());
		EXPECT_EQ(3, unit->getHealthLost());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->initUnit();

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(0); // damage == 0
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(100, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());
	}

	// CombatUnit::applyHeroStats()
	TEST(CombatUnit, shouldApplyHeroStatsOnlyOnce) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		EXPECT_EQ(1, unit->getBaseStats().atk);
		EXPECT_EQ(1, unit->getBaseStats().def);

		unit->applyHeroStats();

		EXPECT_EQ(3, unit->getBaseStats().atk);
		EXPECT_EQ(2, unit->getBaseStats().def);

		unit->applyHeroStats();

		EXPECT_EQ(3, unit->getBaseStats().atk);
		EXPECT_EQ(2, unit->getBaseStats().def);
	}

	// CombatUnit::getAttackGain()
	TEST(CombatUnit, shouldReturnAttackGainWithOnlyHeroStatsAsModifier) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		EXPECT_EQ(0, unit->getAttackGain());
		unit->applyHeroStats();
		EXPECT_EQ(2, unit->getAttackGain());
	}

	// CombatUnit::getDefenseGain()
	TEST(CombatUnit, shouldReturnDefenseGainWithOnlyHeroStatsAsModifier) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		EXPECT_EQ(0, unit->getDefenseGain());
		unit->applyHeroStats();
		EXPECT_EQ(1, unit->getDefenseGain());
	}

	// CombatUnit::getDefenseGain()
	TEST(CombatUnit, shouldIncreaseDefenseGainByOneIfUnitIsDefending) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		EXPECT_EQ(0, unit->getDefenseGain());
		unit->applyHeroStats();
		EXPECT_EQ(1, unit->getDefenseGain());
		unit->defend();
		EXPECT_EQ(2, unit->getDefenseGain());
	}

	// CombatUnit::getBaseAverageDmg()
	TEST(CombatUnit, shouldReturnBaseAverageDmgForMeleeUnit) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_FLOAT_EQ(1.0f, unit->getBaseAverageDmg());

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_FLOAT_EQ(1.5f, unit->getBaseAverageDmg());
	}

	// CombatUnit::getFightValuePerOneHP()
	TEST(CombatUnit, shouldReturnUnitFightValuePerOneHpNoMatterModifiersAndStats) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(15, unit->getFightValuePerOneHp());

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());

		unit->applyDamage(5);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());
	}

	// CombatUnit::getFightValuePerUnitStack()
	TEST(CombatUnit, shouldReturnStackUnitFightValueNoMatterModifiers) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(1500, unit->getFightValuePerUnitStack());

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2487.5, unit->getFightValuePerUnitStack());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2475, unit->getFightValuePerUnitStack());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2462.5, unit->getFightValuePerUnitStack());
	}

	// CombatUnits::getUnitStackHP()
	TEST(CombatUnit, shouldGetUnitStackHPReturnCorrectValues) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(100, unit->getUnitStackHP());

		hero = createHero(createArmy("Imp", 0));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(0, unit->getUnitStackHP());

		hero = createHero(createArmy("Goblin", 11));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(55, unit->getUnitStackHP());

		hero = createHero(createArmy("Imp", 1));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyDamage(2);
		EXPECT_EQ(2, unit->getUnitStackHP());

		hero = createHero(createArmy("Goblin", 11));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyDamage(1);
		EXPECT_EQ(54, unit->getUnitStackHP());
	}

	// CombatUnit::getFightValue()
	TEST(CombatUnit, shouldReturnSingleUnitFightValueNoMatterTheStats) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		EXPECT_EQ(15, unit->getFightValue());

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyDamage(2);
		EXPECT_EQ(50, unit->getFightValue());
	}

}