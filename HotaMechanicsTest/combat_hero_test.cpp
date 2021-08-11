#include "gtest/gtest.h"

#include "utils.h"

namespace CombatHeroTest {
	using namespace HotaMechanics;
	using namespace TestUtils;

	// CombatHero::getUnits()
	TEST(CombatHero, shouldGetUnitPointersToUnitsGeneratedAtHeroInitialization) {
		// hero initialization
		auto hero = createHero(createArmy("Peasant", 500, "Imp", 200, "Goblin", 30, "Gremlin", 2));
		auto units = hero.getUnitsPtrs();
		EXPECT_EQ(4, units.size());
		std::for_each(std::begin(units), std::end(units), [&hero](const auto _unit_ptr) { EXPECT_TRUE(_unit_ptr->getHero() == &hero); });

		// hero move
		CombatHero moved_hero(createHero(createArmy("Imp", 50, "Goblin", 30)));
		units = moved_hero.getUnitsPtrs();
		EXPECT_EQ(2, units.size());
		std::for_each(std::begin(units), std::end(units), [&moved_hero](const auto _unit_ptr) { EXPECT_TRUE(_unit_ptr->getHero() == &moved_hero); });
	
		// hero copy
		CombatHero copy_hero(moved_hero);
		auto& units_ptrs = copy_hero.getUnitsPtrs();
		EXPECT_EQ(2, units_ptrs.size());
		std::for_each(std::begin(units_ptrs), std::end(units_ptrs), [&copy_hero](const auto _unit_ptr) { EXPECT_TRUE(_unit_ptr->getHero() == &copy_hero); });

		// copied hero should create new units based on hero being copied
		auto moved_hero_units = moved_hero.getUnitsPtrs();
		auto copy_hero_units = copy_hero.getUnitsPtrs();
		EXPECT_EQ(moved_hero_units.size(), copy_hero_units.size());
		for (auto i : range(moved_hero_units.size())) {
			EXPECT_NE(moved_hero_units[i], copy_hero_units[i]);
			EXPECT_NE(moved_hero_units[i]->getHero(), copy_hero_units[i]->getHero());
		}
	}

	// CombatHero::getUnitId()
	TEST(CombatHero, shouldReturnUnitIdxInUnitsVector) {
		auto hero = createHero(createArmy("Peasant", 500, "Imp", 200, "Goblin", 30, "Gremlin", 2));
		auto units = hero.getUnitsPtrs();
		for (auto i : range(units.size())) {
			EXPECT_EQ(i, hero.getUnitId(*units[i]));
		}
	}
	
	// CombatHero::isAlive()
	TEST(CombatHero, shouldReturnTrueIfAnyOfUnitsAreAliveFalseOtherwise) {
		auto hero = createHero(createArmy("Peasant", 500, "Imp", 200));
		EXPECT_TRUE(hero.isAlive());

		// kill peasants
		const_cast<CombatUnit*>(hero.getUnitsPtrs()[0])->applyDamage(666);
		EXPECT_TRUE(hero.isAlive()); // still imps alive

		// kill imps
		const_cast<CombatUnit*>(hero.getUnitsPtrs()[1])->applyDamage(2137);
		EXPECT_FALSE(hero.isAlive()); // no units alive
	}
}; // CombatHeroTest