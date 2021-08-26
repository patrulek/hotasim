#include "gtest/gtest.h"

#include "utils.h"

namespace CombatHeroTest {
	using namespace HotaMechanics;
	using namespace TestUtils;


	class CombatHeroTest : public ::testing::Test {
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

	// CombatHero::getUnits()
	TEST_F(CombatHeroTest, shouldGetUnitPointersToUnitsGeneratedAtHeroInitialization) {
		// hero initialization
		SetUp(createArmy("Peasant", 500, "Imp", 200, "Goblin", 30, "Gremlin", 2));
		auto& units = hero->getUnitsPtrs();
		EXPECT_EQ(4, units.size());
		std::for_each(std::begin(units), std::end(units), [this](const auto _unit_ptr) { EXPECT_TRUE(&_unit_ptr->getHero() == this->hero.get()); });

		// hero move
		CombatHero moved_hero(createHero(createArmy("Imp", 50, "Goblin", 30)));
		auto& units2 = moved_hero.getUnitsPtrs();
		EXPECT_EQ(2, units2.size());
		std::for_each(std::begin(units2), std::end(units2), [&moved_hero](const auto _unit_ptr) { EXPECT_TRUE(&_unit_ptr->getHero() == &moved_hero); });
	
		// hero copy
		CombatHero copy_hero(moved_hero);
		auto& units3 = copy_hero.getUnitsPtrs();
		EXPECT_EQ(2, units3.size());
		std::for_each(std::begin(units3), std::end(units3), [&copy_hero](const auto _unit_ptr) { EXPECT_TRUE(&_unit_ptr->getHero() == &copy_hero); });

		// copied hero should create new units based on hero being copied
		auto& moved_hero_units = moved_hero.getUnitsPtrs();
		auto& copy_hero_units = copy_hero.getUnitsPtrs();
		EXPECT_EQ(moved_hero_units.size(), copy_hero_units.size());
		for (auto i : range(moved_hero_units.size())) {
			EXPECT_NE(moved_hero_units[i], copy_hero_units[i]);
			EXPECT_NE(&moved_hero_units[i]->getHero(), &copy_hero_units[i]->getHero());
		}
	}

	// CombatHero::getUnitId()
	TEST_F(CombatHeroTest, shouldReturnUnitIdxInUnitsVector) {
		SetUp(createArmy("Peasant", 500, "Imp", 200, "Goblin", 30, "Gremlin", 2));
		auto& units = hero->getUnitsPtrs();
		for (auto i : range(units.size())) {
			EXPECT_EQ(units[i]->getUnitId(), hero->getUnitId(*units[i]));
		}
	}
	
	// CombatHero::isAlive()
	TEST_F(CombatHeroTest, shouldReturnTrueIfAnyOfUnitsAreAliveFalseOtherwise) {
		SetUp(createArmy("Peasant", 500, "Imp", 200));
		EXPECT_TRUE(hero->isAlive());

		// kill peasants
		const_cast<CombatUnit*>(hero->getUnitsPtrs()[0])->applyDamage(666);
		EXPECT_TRUE(hero->isAlive()); // still imps alive

		// kill imps
		const_cast<CombatUnit*>(hero->getUnitsPtrs()[1])->applyDamage(2137);
		EXPECT_FALSE(hero->isAlive()); // no units alive
	}

	TEST_F(CombatHeroTest, shouldEqualHeroesReturnSameHashes) {
		SetUp(createArmy("Peasant", 500, "Imp", 200));
		Hash h1 = hero->rehash();
		auto h_ptr = hero.get();

		CombatHero hero2(*hero_template, *permutation, CombatSide::ATTACKER);
		Hash h2 = hero2.rehash();
		auto h_ptr2 = &hero2;

		EXPECT_EQ(h1, h2);
		EXPECT_NE(h_ptr, h_ptr2);
	}

	TEST_F(CombatHeroTest, shouldHeroesWithDifferentStateReturnDifferentHashes) {
		SetUp(createArmy("Peasant", 500, "Imp", 200));
		Hash h1 = hero->rehash();

		SetUp(createArmy("Peasant", 500, "Imp", 200), 2, 1);
		Hash h2 = hero->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 500, "Imp", 200), 0, 0, CombatSide::DEFENDER);
		h2 = hero->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 500, "Imp", 200));
		const_cast<CombatUnit*>(hero->getUnitsPtrs()[1])->applyDamage(1);
		h2 = hero->rehash();

		EXPECT_NE(h1, h2);

		SetUp(createArmy("Peasant", 500, "Imp", 200));
		const_cast<PrimaryStats&>(hero->getStats().primary_stats).mana = 5;
		h2 = hero->rehash();
		
		EXPECT_NE(h1, h2);
	}
}; // CombatHeroTest