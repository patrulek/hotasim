#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_calculator.h"
#include "../HotaMechanics/combat_pathfinder.h"

#include "utils.h"

namespace CombatCalculatorTest {
	using namespace HotaMechanics;
	using namespace TestUtils;
	using namespace HotaMechanics::Calculator;

	// CombatCalculator::calculateMeleeUnitAverageDamage(attacker, defender)
	TEST(CombatCalculator, shouldReturnCorrectMeleeUnitDmgWithoutAnyModifiers) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());

		// 200 imps should do 315 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(315, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 475 dmg on average to 200 imps without any attack/defense and/or spells modifiers
		EXPECT_EQ(475, calculateMeleeUnitAverageDamage(*unit2, *unit));


		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		// 100 imps should do 157 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(157, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 400 dmg on average to 100 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(400, calculateMeleeUnitAverageDamage(*unit2, *unit));

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		// 50 imps should do 78 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(78, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 200 dmg on average to 50 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(200, calculateMeleeUnitAverageDamage(*unit2, *unit));

		hero = createHero(createArmy("Goblin", 30));
		unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		// 30 goblins should do 51 dmg on average to peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(51, calculateMeleeUnitAverageDamage(*unit, *unit2));
	}

	// CombatCalculator::calculateCounterAttackMeleeUnitAverageDamage(attacker, defender)
	TEST(CombatCalculator, shouldReturnZeroForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsKilledOrCantRetaliate) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		auto hero2 = createHero(createArmy("Peasant", 100));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->resetState();

		// 200 imps should kill all peasants, so 0 dmg in counterattack
		EXPECT_EQ(0, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));

		hero2 = createHero(createArmy("Peasant", 500));
		unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->setRetaliated();

		// 200 imps will not kill all peasants, but they already retaliated so 0 dmg
		EXPECT_EQ(0, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));
	}

	// CombatCalculator::calculateCounterAttackMeleeUnitAverageDamage(attacker, defender)
	TEST(CombatCalculator, shouldReturnCorrectValueForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsNotKilledAndCanRetaliate) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->resetState();

		// 200 imps should kill 315 peasants, so 185 peasants should retaliate 175 dmg to imps
		EXPECT_EQ(175, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should kill 118 imps, so 82 imps should retaliate 129 to peasants
		EXPECT_EQ(129, calculateCounterAttackMeleeUnitAverageDamage(*unit2, *unit));
	}

	// CombatCalculator::calculateUnitFightValueModifier(unit); calculateUnitAttack/DefenseFightValueModifier(unit)
	TEST(CombatCalculator, shouldReturn1IfNoOtherModifiersAndEqualStats) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		EXPECT_FLOAT_EQ(1.0f, calculateUnitAttackFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.0f, calculateUnitDefenceFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.0f, calculateUnitFightValueModifier(*unit));
	}


	// CombatCalculator::calculateUnitFightValueModifier(unit); calculateUnitAttack/DefenseFightValueModifier(unit)
	TEST(CombatCalculator, shouldReturnGreaterThan1IfNoOtherModifiersAndAppliedHeroStats) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());
		unit->applyHeroStats();

		EXPECT_FLOAT_EQ(1.1f, calculateUnitAttackFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.05f, calculateUnitDefenceFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.074709f, calculateUnitFightValueModifier(*unit));
	}

	// CombatCalculator::calculateStackUnitFightValue(unit)
	TEST(CombatCalculator, shouldReturnUnitFightValueModifierTimesStackUnitFightValue) {
		auto hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		EXPECT_EQ(1500, calculateStackUnitFightValue(*unit)); // stack unit fight value for base stats
		unit->applyHeroStats();
		EXPECT_EQ(1612, calculateStackUnitFightValue(*unit)); // stack unit fight value after applying hero stats to unit stats
	}

	// CombatCalculator::calculateBaseHeroFightValue(hero)
	TEST(CombatCalculator, DISABLED_shouldReturnBaseHeroFightValue) {
		EXPECT_TRUE(false);
	}

	// CombatCalculator::calculateHeroFightValue(hero)
	TEST(CombatCalculator, shouldReturnSummedUnitFightValuesForHero) {
		auto& hero = createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1);
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		EXPECT_EQ(1500, calculateHeroFightValue(hero)); // all hero units without modifiers
		unit->applyHeroStats();
		EXPECT_EQ(1612, calculateHeroFightValue(hero)); // all hero units with hero stats applied

		hero = createHero(createArmy("Peasant", 100, "Peasant", 49), CombatSide::ATTACKER, 2, 1);
		EXPECT_EQ(1500 + 735, calculateBaseHeroFightValue(hero)); // all hero units without modifiers
		for (auto unit : hero.getUnitsPtrs())
			const_cast<CombatUnit*>(unit)->applyHeroStats();
		EXPECT_EQ(1612 + 789, calculateHeroFightValue(hero)); // all hero units with hero stats applied
	}

	// CombatCalculator::calculateFightValueAdvantageAfterMeleeUnitAttack(attacker, defender)
	TEST(CombatCalculator, shouldReturnCorrectFightValueGainForMeleeUnitAttackWithoutAnyModifiersWhenSimilarArmyStrength) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());

		// fight value gain when 200 imps attack 500 peasants first
		EXPECT_EQ(2538, calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, *unit2));

		// fight value gain when 500 peasants attack 200 imps first
		EXPECT_EQ(4002, calculateFightValueAdvantageAfterMeleeUnitAttack(*unit2, *unit));
	}

	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST(CombatCalculator, shouldReturnOnlyZeroFightValueGainOnHexesForMeleeUnitWhenAttackerWeakerThanDefender) {
		auto hero = createHero(createArmy("Imp", 50));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->moveTo(getHexId(8, 2));

		CombatField field(createField());
		CombatPathfinder pathfinder;
		field.fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field.fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

		std::array<int, FIELD_SIZE> expected;
		expected.fill(0);
		
		// imp fight value gain is lesser than 0 (peasants do more on retaliation than imps) so no fields are set
		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, hero, field, pathfinder));
	}

	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST(CombatCalculator, shouldReturnCorrectFightValueGainOnHexesForMeleeUnitWhenAttackerLittleStrongerThanDefender) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());
		unit->moveTo(getHexId(5, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->moveTo(getHexId(5, 15));

		CombatField field(createField());
		CombatPathfinder pathfinder;
		field.fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field.fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

		int fv = -2538; // this is the imp fight value gain when they attack first
		std::array<int, FIELD_SIZE> expected{
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

		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, hero, field, pathfinder));
	}


	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST(CombatCalculator, DISABLED_shouldReturnMaximumFightValueGainOnHexesForMeleeUnitWhenAttackerIncrediblyStrongerThanDefender) {
		auto hero = createHero(createArmy("Imp", 301));
		auto unit = const_cast<CombatUnit*>(hero.getUnitsPtrs().front());
		unit->moveTo(getHexId(5, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnitsPtrs().front());
		unit2->moveTo(getHexId(5, 15));

		CombatField field(createField());
		CombatPathfinder pathfinder;
		field.fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field.fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

		// 301 imps fight value is > 2 * 500 peasants fight value; 
		int fv = -7500; // TODO: implement case when attacker fight value > 2 * defender fight value; check if there isnt case for 5 * also
		std::array<int, FIELD_SIZE> expected{
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
		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, hero, field, pathfinder));
	}
}; // CombatCalculatorTest