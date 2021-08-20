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

	class CombatCalculatorTest : public ::testing::Test {
	protected:
		void SetUp() override {
			pathfinder = std::make_shared<CombatPathfinder>();
			field = std::make_shared<CombatField>(createField());
		}

		void setAttacker(CombatHero&& _hero) {
			attacker = std::make_shared<CombatHero>(std::move(_hero));
			unit = const_cast<CombatUnit*>(attacker->getUnitsPtrs()[0]);
		}
		void setDefender(CombatHero&& _hero) {
			defender = std::make_shared<CombatHero>(std::move(_hero));
			unit2 = const_cast<CombatUnit*>(defender->getUnitsPtrs()[0]);
		}

		std::shared_ptr<CombatHero> attacker{ nullptr };
		std::shared_ptr<CombatHero> defender{ nullptr };
		std::shared_ptr<CombatPathfinder> pathfinder{ nullptr };
		std::shared_ptr<CombatField> field{ nullptr };
		CombatUnit* unit{ nullptr };
		CombatUnit* unit2{ nullptr };
	};

	// CombatCalculator::calculateMeleeUnitAverageDamage(attacker, defender)
	TEST_F(CombatCalculatorTest, shouldReturnCorrectMeleeUnitDmgWithoutAnyModifiers) {
		setAttacker(createHero(createArmy("Imp", 200)));
		setDefender(createHero(createArmy("Peasant", 500)));

		// 200 imps should do 315 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(315, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 475 dmg on average to 200 imps without any attack/defense and/or spells modifiers
		EXPECT_EQ(475, calculateMeleeUnitAverageDamage(*unit2, *unit));

		setAttacker(createHero(createArmy("Imp", 100)));

		// 100 imps should do 157 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(157, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 400 dmg on average to 100 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(400, calculateMeleeUnitAverageDamage(*unit2, *unit));

		setAttacker(createHero(createArmy("Imp", 50)));

		// 50 imps should do 78 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(78, calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 200 dmg on average to 50 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(200, calculateMeleeUnitAverageDamage(*unit2, *unit));

		setAttacker(createHero(createArmy("Goblin", 30)));

		// 30 goblins should do 51 dmg on average to peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(51, calculateMeleeUnitAverageDamage(*unit, *unit2));
	}

	// CombatCalculator::calculateCounterAttackMeleeUnitAverageDamage(attacker, defender)
	TEST_F(CombatCalculatorTest, shouldReturnZeroForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsKilledOrCantRetaliate) {
		setAttacker(createHero(createArmy("Imp", 200)));
		setDefender(createHero(createArmy("Peasant", 100)));

		// 200 imps should kill all peasants, so 0 dmg in counterattack
		EXPECT_EQ(0, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));

		setDefender(createHero(createArmy("Peasant", 500)));
		unit2->setRetaliated();

		// 200 imps will not kill all peasants, but they already retaliated so 0 dmg
		EXPECT_EQ(0, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));
	}

	// CombatCalculator::calculateCounterAttackMeleeUnitAverageDamage(attacker, defender)
	TEST_F(CombatCalculatorTest, shouldReturnCorrectValueForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsNotKilledAndCanRetaliate) {
		setAttacker(createHero(createArmy("Imp", 200)));
		setDefender(createHero(createArmy("Peasant", 500)));

		// 200 imps should kill 315 peasants, so 185 peasants should retaliate 175 dmg to imps
		EXPECT_EQ(175, calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should kill 118 imps, so 82 imps should retaliate 129 to peasants
		EXPECT_EQ(129, calculateCounterAttackMeleeUnitAverageDamage(*unit2, *unit));
	}

	// CombatCalculator::calculateUnitFightValueModifier(unit); calculateUnitAttack/DefenseFightValueModifier(unit)
	TEST_F(CombatCalculatorTest, shouldReturn1IfNoOtherModifiersAndEqualStats) {
		setAttacker(createHero(createArmy("Peasant", 100)));

		EXPECT_FLOAT_EQ(1.0f, calculateUnitAttackFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.0f, calculateUnitDefenceFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.0f, calculateUnitFightValueModifier(*unit));
	}


	// CombatCalculator::calculateUnitFightValueModifier(unit); calculateUnitAttack/DefenseFightValueModifier(unit)
	TEST_F(CombatCalculatorTest, shouldReturnGreaterThan1IfNoOtherModifiersAndAppliedHeroStats) {
		setAttacker(createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1));
		unit->applyHeroStats();

		EXPECT_FLOAT_EQ(1.1f, calculateUnitAttackFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.05f, calculateUnitDefenceFightValueModifier(*unit));
		EXPECT_FLOAT_EQ(1.074709f, calculateUnitFightValueModifier(*unit));
	}

	// CombatCalculator::calculateStackUnitFightValue(unit)
	TEST_F(CombatCalculatorTest, shouldReturnUnitFightValueModifierTimesStackUnitFightValue) {
		setAttacker(createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1));

		EXPECT_EQ(1500, calculateStackUnitFightValue(*unit)); // stack unit fight value for base stats
		unit->applyHeroStats();
		EXPECT_EQ(1612, calculateStackUnitFightValue(*unit)); // stack unit fight value after applying hero stats to unit stats
	}

	// CombatCalculator::calculateBaseHeroFightValue(hero)
	TEST_F(CombatCalculatorTest, DISABLED_shouldReturnBaseHeroFightValue) {
		EXPECT_TRUE(false);
	}

	// CombatCalculator::calculateHeroFightValue(hero)
	TEST_F(CombatCalculatorTest, shouldReturnSummedUnitFightValuesForHero) {
		setAttacker(createHero(createArmy("Peasant", 100), CombatSide::ATTACKER, 2, 1));

		EXPECT_EQ(1500, calculateHeroFightValue(*attacker)); // all hero units without modifiers
		unit->applyHeroStats();
		EXPECT_EQ(1612, calculateHeroFightValue(*attacker)); // all hero units with hero stats applied

		setAttacker(createHero(createArmy("Peasant", 100, "Peasant", 49), CombatSide::ATTACKER, 2, 1));
		EXPECT_EQ(1500 + 735, calculateBaseHeroFightValue(*attacker)); // all hero units without modifiers
		for (auto unit : attacker->getUnitsPtrs())
			const_cast<CombatUnit*>(unit)->applyHeroStats();
		EXPECT_EQ(1612 + 789, calculateHeroFightValue(*attacker)); // all hero units with hero stats applied
	}

	// CombatCalculator::calculateFightValueAdvantageAfterMeleeUnitAttack(attacker, defender)
	TEST_F(CombatCalculatorTest, shouldReturnCorrectFightValueGainForMeleeUnitAttackWithoutAnyModifiersWhenSimilarArmyStrength) {
		setAttacker(createHero(createArmy("Imp", 200)));
		setDefender(createHero(createArmy("Peasant", 500)));

		// fight value gain when 200 imps attack 500 peasants first
		EXPECT_EQ(2538, calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, *unit2));

		// fight value gain when 500 peasants attack 200 imps first
		EXPECT_EQ(4002, calculateFightValueAdvantageAfterMeleeUnitAttack(*unit2, *unit));
	}

	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST_F(CombatCalculatorTest, shouldReturnOnlyZeroFightValueGainOnHexesForMeleeUnitWhenAttackerWeakerThanDefender) {
		setAttacker(createHero(createArmy("Imp", 50)));
		unit->moveTo(getHexId(8, 1));

		setDefender(createHero(createArmy("Peasant", 500)));
		unit2->moveTo(getHexId(8, 15));

		field->fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field->fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

		std::array<int, FIELD_SIZE> expected; expected.fill(0);
		
		// imp fight value gain is lesser than 0 (peasants do more on retaliation than imps) so no fields are set
		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, *attacker, *field, *pathfinder));
	}

	TEST_F(CombatCalculatorTest, DISABLED_shouldReturnZeroIfAttackerStrongerButAlreadyInAttackRange) {
		EXPECT_TRUE(false); // todo
	}

	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST_F(CombatCalculatorTest, shouldReturnCorrectFightValueGainOnHexesForMeleeUnitWhenAttackerLittleStrongerThanDefender) {
		setAttacker(createHero(createArmy("Imp", 200)));
		unit->moveTo(getHexId(5, 1));

		setDefender(createHero(createArmy("Peasant", 500)));
		unit2->moveTo(getHexId(5, 15));

		field->fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field->fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

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

		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, *attacker, *field, *pathfinder));
	}


	// CombatCalculator::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST_F(CombatCalculatorTest, DISABLED_shouldReturnMaximumFightValueGainOnHexesForMeleeUnitWhenAttackerIncrediblyStrongerThanDefender) {
		// todo
		setAttacker(createHero(createArmy("Imp", 301)));
		unit->moveTo(getHexId(5, 1));

		setDefender(createHero(createArmy("Peasant", 500)));
		unit2->moveTo(getHexId(5, 15));

		field->fillHex(unit->getHex(), CombatHexOccupation::UNIT);
		field->fillHex(unit2->getHex(), CombatHexOccupation::UNIT);

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
		EXPECT_EQ(expected, calculateFightValueAdvantageOnHexes(*unit2, *attacker, *field, *pathfinder));
	}
}; // CombatCalculatorTest