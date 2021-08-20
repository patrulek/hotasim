#include "gtest/gtest.h"

#include <unordered_map>

#include "../HotaMechanics/utils.h"
#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_field.h"
#include "utils.h"

namespace CombatManagerTest {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Utils;
	using namespace TestUtils;

	class CombatManagerTest : public ::testing::Test {
	protected:
		void SetUp() override {
			manager.reset(createCombatManager());
			internalSetup();
		}

		void SetUp(CombatHero&& _attacker, CombatHero&& _defender, const bool _initialize = true) {
			manager.reset(createCombatManager(_attacker, _defender, _initialize));
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

	// CombatManager::nextState
	TEST_F(CombatManagerTest, shouldThrowWhenNoInitializedOrUnitMove) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER), false);
		// createCombatManager dont call initialize now
		EXPECT_ANY_THROW(manager->nextState());

		manager->initialize(); 
		manager->nextState(); // start battle;
		CombatAction defend_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		manager->nextStateByAction(defend_action);

		EXPECT_ANY_THROW(manager->nextState()); // its unit move now, so we should provide unit action and call nextStateByAction

		// need mock to test more
	}

	// CombatManager::nextStateByAction(action)
	TEST_F(CombatManagerTest, shouldThrowWhenNoInitialized) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER), false);

		// createCombatManager dont call initialize now
		CombatAction defend_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		EXPECT_ANY_THROW(manager->nextStateByAction(defend_action));

		// need mock to test more
	}

	// CombatManager::generateActionsForPlayer()
	TEST_F(CombatManagerTest, shouldInitializePlaceUnitsAtStartPosition) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed
		auto& current_state = manager->getCurrentState();
		auto& attacker_units = current_state.attacker.getUnitsPtrs();
		auto& defender_units = current_state.defender.getUnitsPtrs();

		std::for_each(std::begin(attacker_units), std::end(attacker_units), [](const auto _obj) { EXPECT_NE(INVALID_HEX_ID, _obj->getHex()); });
		std::for_each(std::begin(defender_units), std::end(defender_units), [](const auto _obj) { EXPECT_NE(INVALID_HEX_ID, _obj->getHex()); });
	}

	// CombatManager::isUnitMove()
	TEST_F(CombatManagerTest, shouldReturnTrueIfUnitMoveFalseOtherwise) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(manager->isUnitMove()); // battle didnt start so theres no unit move yet
		manager->nextState(); // start battle
		EXPECT_TRUE(manager->isUnitMove()); // now it is unit move;
		
		CombatAction defend_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		manager->nextStateByAction(defend_action);
		EXPECT_TRUE(manager->isUnitMove()); // first unit in turn defended, but there is another unit in this turn to move
		manager->nextStateByAction(defend_action);
		EXPECT_FALSE(manager->isUnitMove()); // all units in turn moved so there is no unit move now
		manager->nextState(); // start another turn
		EXPECT_TRUE(manager->isUnitMove()); // now it is unit move again
	}

	// CombatManager::isPlayerMove()
	TEST_F(CombatManagerTest, shouldReturnTrueIfPlayerMoveFalseOtherwise) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(manager->isPlayerMove()); // battle didnt start so theres no player move yet
		manager->nextState(); // start battle
		EXPECT_TRUE(manager->isPlayerMove()); // now it is player move (as he starts first with this configuration);

		CombatAction defend_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		manager->nextStateByAction(defend_action);
		EXPECT_FALSE(manager->isPlayerMove()); // no, there is ai move now
		manager->nextStateByAction(defend_action);
		EXPECT_FALSE(manager->isPlayerMove()); // all units in turn moved so there is battle move now
		manager->nextState(); // start another turn
		EXPECT_TRUE(manager->isPlayerMove()); // now it is player move again
	}

	// CombatManager::isCombatFinished()
	TEST_F(CombatManagerTest, shouldReturnTrueIfCombatFinishedFalseOtherwise) {
		SetUp(createHero(createArmy("Peasant", 200)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(manager->isCombatFinished()); // battle didnt start so cant be finishd
		manager->nextState(); // start battle
		EXPECT_FALSE(manager->isCombatFinished()); // any unit from both sides still alive

		CombatAction defend_action{ CombatActionType::DEFENSE, 0xFF, 0xFF, true };
		manager->nextStateByAction(defend_action);
		EXPECT_FALSE(manager->isCombatFinished()); // any unit from both sides still alive
		manager->nextStateByAction(defend_action);
		EXPECT_FALSE(manager->isCombatFinished()); // any unit from both sides still alive
		manager->nextState(); // start another turn
		EXPECT_FALSE(manager->isCombatFinished()); // any unit from both sides still alive
		
		CombatAction kill_action{ CombatActionType::ATTACK, 0, 99, true };
		manager->nextStateByAction(kill_action); // player unit attacks ai unit and kill it
		EXPECT_TRUE(manager->isCombatFinished()); // this ends battle because no units left in ai hero
	}
}