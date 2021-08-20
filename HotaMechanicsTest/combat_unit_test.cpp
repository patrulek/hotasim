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
	using namespace Constants;

	class CombatUnitTest : public ::testing::Test {
	protected:
		void SetUp(const std::vector<UnitStack>& _army, const int _atk = 0, const int _def = 0, const CombatSide _side = CombatSide::ATTACKER) {
			if (hero) {
				hero_template.reset();
				permutation.reset();
				hero.reset();
			}

			hero_template = std::make_shared<Hero>();
			hero_template->army = _army;
			hero_template->stats.base_stats.atk = _atk;
			hero_template->stats.base_stats.def = _def;

			ArmyPermutation perm = Utils::createBaseArmyPermutation(hero_template->army);
			permutation = std::make_shared<ArmyPermutation>(std::move(perm));

			hero = std::make_shared<CombatHero>(*hero_template, *permutation, _side);
			unit = const_cast<CombatUnit*>(hero->getUnitsPtrs().front());
		}

		std::shared_ptr<Hero> hero_template{ nullptr };
		std::shared_ptr<ArmyPermutation> permutation{ nullptr };
		std::shared_ptr<CombatHero> hero{ nullptr };
		CombatUnit* unit;
	};


	// CombatUnit::applyDamage(damage >= CombatUnit::getStackHP())
	TEST_F(CombatUnitTest, shouldKillStackIfDmgGreaterEqualStackHp) {
		SetUp(createArmy("Peasant", 500));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(500); // damage == CombatUnit::getStackHP()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		SetUp(createArmy("Peasant", 500));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501); // damage > CombatUnit::getStackHP()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		SetUp(createArmy("Imp", 100));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(501); // damage > CombatUnit::getStack()
		EXPECT_FALSE(unit->isAlive());
		EXPECT_EQ(0, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());
	}

	// CombatUnit::applyDamage(damage >= 0 && damage < CombatUnit::getStackHP())
	TEST_F(CombatUnitTest, shouldSpareSomeUnitsIfDmgLesserThanStackHP) {
		SetUp(createArmy("Peasant", 500));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(300); // damage > 0 && damage < CombatUnit::getStackHP()
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(200, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		SetUp(createArmy("Imp", 100));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(20); // damage == 5 * primary_stats.hp 
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());

		SetUp(createArmy("Imp", 100));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(23); // damage > 0 && damage < CombatUnit::getStackHP()
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(95, unit->getStackNumber());
		EXPECT_EQ(3, unit->getHealthLost());
		
		SetUp(createArmy("Imp", 100));

		EXPECT_TRUE(unit->isAlive());
		unit->applyDamage(0); // damage == 0
		EXPECT_TRUE(unit->isAlive());
		EXPECT_EQ(100, unit->getStackNumber());
		EXPECT_EQ(0, unit->getHealthLost());
	}

	// CombatUnit::applyHeroStats()
	TEST_F(CombatUnitTest, shouldApplyHeroStatsOnlyOnce) {
		SetUp(createArmy("Peasant", 100), 2, 1);
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
	TEST_F(CombatUnitTest, shouldReturnAttackGainWithOnlyHeroStatsAsModifier) {
		SetUp(createArmy("Peasant", 100), 2, 1);

		EXPECT_EQ(0, unit->getAttackGain());
		unit->applyHeroStats();
		EXPECT_EQ(2, unit->getAttackGain());
	}

	// CombatUnit::getDefenseGain()
	TEST_F(CombatUnitTest, shouldReturnDefenseGainWithOnlyHeroStatsAsModifier) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		EXPECT_EQ(0, unit->getDefenseGain());
		unit->applyHeroStats();
		EXPECT_EQ(1, unit->getDefenseGain());
	}

	// CombatUnit::getDefenseGain()
	TEST_F(CombatUnitTest, shouldIncreaseDefenseGainByOneIfUnitIsDefending) {
		SetUp(createArmy("Peasant", 100), 2, 1);

		EXPECT_EQ(0, unit->getDefenseGain());
		unit->applyHeroStats();
		EXPECT_EQ(1, unit->getDefenseGain());
		unit->defend();
		EXPECT_EQ(2, unit->getDefenseGain());
	}

	// CombatUnit::getBaseAverageDmg()
	TEST_F(CombatUnitTest, shouldReturnBaseAverageDmgForMeleeUnit) {
		SetUp(createArmy("Peasant", 100));
		EXPECT_FLOAT_EQ(1.0f, unit->getBaseAverageDmg());
		SetUp(createArmy("Imp", 100));
		EXPECT_FLOAT_EQ(1.5f, unit->getBaseAverageDmg());
	}

	// CombatUnit::getFightValuePerOneHP()
	TEST_F(CombatUnitTest, shouldReturnUnitFightValuePerOneHpNoMatterModifiersAndStats) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		EXPECT_EQ(15, unit->getFightValuePerOneHp());

		SetUp(createArmy("Imp", 50));
		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());

		unit->applyDamage(5);
		EXPECT_FLOAT_EQ(12.5, unit->getFightValuePerOneHp());
	}

	// CombatUnit::getFightValuePerUnitStack()
	TEST_F(CombatUnitTest, shouldReturnStackUnitFightValueNoMatterModifiers) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		EXPECT_EQ(1500, unit->getFightValuePerUnitStack());

		SetUp(createArmy("Imp", 50));
		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2487.5, unit->getFightValuePerUnitStack());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2475, unit->getFightValuePerUnitStack());

		unit->applyDamage(1);
		EXPECT_FLOAT_EQ(2462.5, unit->getFightValuePerUnitStack());
	}

	// CombatUnits::getUnitStackHP()
	TEST_F(CombatUnitTest, shouldGetUnitStackHPReturnCorrectValues) {
		SetUp(createArmy("Peasant", 100));
		EXPECT_EQ(100, unit->getUnitStackHP());

		SetUp(createArmy("Imp", 0));
		EXPECT_EQ(0, unit->getUnitStackHP());

		SetUp(createArmy("Goblin", 11));
		EXPECT_EQ(55, unit->getUnitStackHP());

		SetUp(createArmy("Imp", 1));
		unit->applyDamage(2);
		EXPECT_EQ(2, unit->getUnitStackHP());

		SetUp(createArmy("Goblin", 11));
		unit->applyDamage(1);
		EXPECT_EQ(54, unit->getUnitStackHP());
	}

	// CombatUnit::getFightValue()
	TEST_F(CombatUnitTest, shouldReturnSingleUnitFightValueNoMatterTheStats) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		EXPECT_EQ(15, unit->getFightValue());
		SetUp(createArmy("Imp", 50));
		unit->applyDamage(2);
		EXPECT_EQ(50, unit->getFightValue());
	}

	TEST_F(CombatUnitTest, shouldRehashTwoEqualUnitsCreateSameHashes) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		Hash h1 = unit->rehash();
		SetUp(createArmy("Peasant", 100), 2, 1);
		Hash h2 = unit->rehash();

		EXPECT_EQ(h1, h2);
	}

	TEST_F(CombatUnitTest, shouldRehashTwoEqualUnitsWithDifferentStatesCreateDifferentHashes) {
		SetUp(createArmy("Peasant", 100), 2, 1);
		Hash h1 = unit->rehash();
		SetUp(createArmy("Peasant", 100), 2, 1);
		unit->wait();
		Hash h2 = unit->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 100), 2, 1);
		unit->moveTo(32);
		h2 = unit->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 100), 2, 1);
		unit->applyDamage(1); // different stack number
		h2 = unit->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 100), 2, 1, CombatSide::DEFENDER);
		h2 = unit->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Imp", 100), 2, 1);
		h1 = unit->rehash();
		SetUp(createArmy("Imp", 100), 2, 1);
		unit->applyDamage(1); // different health lost
		h2 = unit->rehash();

		EXPECT_NE(h1, h2);
	}
}