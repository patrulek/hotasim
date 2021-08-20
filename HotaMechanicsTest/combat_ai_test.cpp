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

	class CombatAITest : public ::testing::Test {
	protected:
		void SetUp() override {
			manager.reset(createCombatManager());
			internalSetup();
		}

		void SetUp(CombatHero&& _attacker, CombatHero&& _defender) {
			manager.reset(createCombatManager(_attacker, _defender));
			internalSetup();
		}

		std::shared_ptr<CombatManager> manager{ nullptr };
		CombatAI* ai{ nullptr };
		CombatPathfinder* pathfinder{ nullptr };
	private:
		void internalSetup() {
			ai = &const_cast<CombatAI&>(manager->getCombatAI());
			pathfinder = &const_cast<CombatPathfinder&>(ai->getPathfinder());
		}
	};

	// CombatAI::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST_F(CombatAITest, DISABLED_shouldSetFightValueGainOnHexes) {
		EXPECT_TRUE(false);
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST_F(CombatAITest, shouldChooseExactlyThisUnitToAttackIfOnlyOneUnitStackLeftInEnemyHero) {
		auto active_stack = manager->getCurrentState().defender.getUnitsPtrs()[0];
		auto unit = manager->getCurrentState().attacker.getUnitsPtrs()[0];
		auto& field = manager->getCurrentState().field;

		std::vector<HexId> hexes{ ai->chooseHexToMoveForAttack(*active_stack, *unit) };
		// only one unit
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker, hexes).size());

		// idx of this unit in enemy hero unit vector
		EXPECT_EQ(0, ai->chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker, hexes).front()->getUnitId());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST_F(CombatAITest, shouldChooseSecondUnitStackToChaseWhenEqualStacksAndCannotAttackBoth) {
		SetUp(createHero(createArmy("Imp", 100, "Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		auto& field = manager->getCurrentState().field;
		field.setTemplate(CombatFieldTemplate::TMP1);
		pathfinder->clearPathCache();
		auto& active_stack = manager->getActiveStack();
		auto& attacker = manager->getCurrentState().attacker;
		auto unit = manager->getCurrentState().attacker.getUnitsPtrs()[0];
		auto unit2 = manager->getCurrentState().attacker.getUnitsPtrs()[1];
		std::vector<HexId> hexes{ ai->chooseHexToMoveForAttack(active_stack, *unit), ai->chooseHexToMoveForAttack(active_stack, *unit2) };

		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack.getHex());
		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 13));
		field.fillHex(active_stack.getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(active_stack, *unit), ai->chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack.getHex());
		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 10));
		field.fillHex(active_stack.getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(active_stack, *unit), ai->chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack.getHex());
		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 7));
		field.fillHex(active_stack.getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(active_stack, *unit), ai->chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack.getHex());
		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 4));
		field.fillHex(active_stack.getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(active_stack, *unit), ai->chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(active_stack, attacker, hexes).front()->getUnitId());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST_F(CombatAITest, shouldChooseRandomlyFromTwoStacksIfEqualStacksAndCanAttackBoth) {
		SetUp(createHero(createArmy("Imp", 100, "Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));

		auto& field = manager->getCurrentState().field;
		field.setTemplate(CombatFieldTemplate::TMP1);
		pathfinder->clearPathCache();

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(6, 3), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 3), true };
		manager->nextStateByAction(player_action);

		auto active_stack = manager->getCurrentState().defender.getUnitsPtrs()[0];
		auto& attacker = manager->getCurrentState().attacker;
		auto unit = manager->getCurrentState().attacker.getUnitsPtrs()[0];
		auto unit2 = manager->getCurrentState().attacker.getUnitsPtrs()[1];

		std::vector<HexId> hexes{ ai->chooseHexToMoveForAttack(*active_stack, *unit), ai->chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack->getHex());
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 13));
		field.fillHex(active_stack->getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnitsPtrs()[0])->moveTo(getHexId(7, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnitsPtrs()[1])->moveTo(getHexId(6, 1));
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(*active_stack, *unit), ai->chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack->getHex());
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 10));
		field.fillHex(active_stack->getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(*active_stack, *unit), ai->chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack->getHex());
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 7));
		field.fillHex(active_stack->getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(*active_stack, *unit), ai->chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai->chooseUnitToAttack(*active_stack, attacker, hexes).front()->getUnitId());

		field.clearHex(active_stack->getHex());
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 4));
		field.fillHex(active_stack->getHex(), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		hexes = std::vector<HexId>{ ai->chooseHexToMoveForAttack(*active_stack, *unit), ai->chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(2, ai->chooseUnitToAttack(*active_stack, attacker, hexes).size());
	}

	// CombatAI::chooseHexToMoveForAttack(_active_stack, _target_unit)
	TEST_F(CombatAITest, shouldChooseCorrectHexWhichMeleeAttackWillBePerformedFrom) {
		SetUp(createHero(createArmy("Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));

		auto& field = manager->getCurrentState().field;
		field.setTemplate(CombatFieldTemplate::TMP1);
		pathfinder->clearPathCache();

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		manager->nextStateByAction(player_action);

		auto active_stack = manager->getCurrentState().defender.getUnitsPtrs()[0];
		ai->calculateFightValueAdvantageOnHexes(*active_stack, manager->getCurrentState().attacker, field);

		EXPECT_EQ(69, ai->chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnitsPtrs()[0]));

		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(4, 12), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 5), true };
		manager->nextStateByAction(player_action);

		EXPECT_EQ(107, ai->chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnitsPtrs()[0]));

		ai_action = CombatAction{ CombatActionType::WALK, 32, getHexId(4, 10), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 3
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(10, 8), true };
		manager->nextStateByAction(player_action);

		EXPECT_EQ(162, ai->chooseHexToMoveForAttack(*active_stack, *manager->getCurrentState().attacker.getUnitsPtrs()[0]));
	}

}; // CombatAITest