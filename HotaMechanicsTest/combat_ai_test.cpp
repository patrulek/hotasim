#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../HotaMechanics/utils.h"
#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "utils.h"

namespace CombatAITest {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Utils;
	using namespace TestUtils;

	// CombatAI::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST(CombatAI, shouldSetFightValueGainOnHexes) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().attacker.getUnits()[0];
		auto& field = manager->getCurrentState().field;

		EXPECT_TRUE(ai.needRecalculateHexesFightValueGain());
		const_cast<CombatAI&>(ai).calculateFightValueAdvantageOnHexes(*active_stack, manager->getCurrentState().defender, field);
		EXPECT_FALSE(ai.needRecalculateHexesFightValueGain());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatAI, shouldChooseExactlyThisUnitToAttackIfOnlyOneUnitStackLeftInEnemyHero) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		auto& field = manager->getCurrentState().field;

		// only one unit
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker).size());

		// idx of this unit in enemy hero unit vector
		EXPECT_EQ(0, ai.chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker).front());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatAI, shouldChooseSecondUnitStackToChaseWhenEqualStacksAndCannotAttackBoth) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(5, 15));

		manager->getCurrentState().attacker = createHero(createArmy("Imp", 100, "Imp", 100));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(2, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[1])->moveTo(getHexId(8, 1));
		auto& attacker = manager->getCurrentState().attacker;

		auto& field = manager->getCurrentState().field;
		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::TMP1));

		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 13));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 10));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 7));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 4));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatCalculator, shouldChooseRandomlyFromTwoStacksIfEqualStacksAndCanAttackBoth) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(5, 15));

		manager->getCurrentState().attacker = createHero(createArmy("Imp", 100, "Imp", 100));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(2, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[1])->moveTo(getHexId(8, 1));
		auto& attacker = manager->getCurrentState().attacker;

		auto& field = manager->getCurrentState().field;
		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::TMP1));

		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 13));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(7, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[1])->moveTo(getHexId(6, 1));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 10));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 7));
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 4));
		EXPECT_EQ(2, ai.chooseUnitToAttack(*active_stack, attacker).size());
	}

	// CombatAI::chooseHexToMoveForAttack(_active_stack, _target_unit)
	TEST(CombatAI, shouldChooseCorrectHexWhichMeleeAttackWillBePerformedFrom) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(5, 15));

		manager->getCurrentState().attacker = createHero(createArmy("Imp", 100));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(5, 1));
		auto& attacker = manager->getCurrentState().attacker;

		auto& field = manager->getCurrentState().field;
		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::TMP1));

		const_cast<CombatAI&>(ai).calculateFightValueAdvantageOnHexes(*active_stack, manager->getCurrentState().attacker, field);

		EXPECT_EQ(69, ai.chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnits()[0]));

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(4, 12));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(7, 5));
		EXPECT_EQ(107, ai.chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnits()[0]));

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(4, 10));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(10, 8));
		EXPECT_EQ(162, ai.chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnits()[0]));
	}

	//  CombatAI::chooseWalkDistanceFromPath(const CombatUnit& _active_stack, int _target_hex, const CombatField& _field) const;
	TEST(CombatAI, DISABLED_shouldReturn0IfNoPathToHex) {
		// if path.size() < unit_speed -> walk_distance = path.size()
		// if path.size() == 0 -> walk_distance = 0
		// if path.size() >= unit_speed -> walk_disntace = (if 'danger zone' then that far, where is safe else unit_speed)

		// need more in-game tests for that test

		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(5, 15));

		manager->getCurrentState().attacker = createHero(createArmy("Imp", 100));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(5, 1));
		auto& attacker = manager->getCurrentState().attacker;

		auto& field = manager->getCurrentState().field;
		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::TMP1));
	}
}; // CombatAITest