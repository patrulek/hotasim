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


	// CombatManager::nextState
	TEST(CombatManager, shouldThrowWhenNoInitializedOrUnitMove) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER), false);
		// createCombatManager dont call initialize now
		EXPECT_ANY_THROW(combat_manager->nextState());

		combat_manager->initialize(); 
		combat_manager->nextState(); // start battle;
		CombatAction action{ CombatActionType::DEFENSE, -1, -1, -1 };
		combat_manager->nextStateByAction(action);

		EXPECT_ANY_THROW(combat_manager->nextState()); // its unit move now, so we should provide unit action and call nextStateByAction

		// need mock to test more
	}

	// CombatManager::nextStateByAction(action)
	TEST(CombatManager, shouldThrowWhenNoInitialized) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER), false);
		// createCombatManager dont call initialize now
		CombatAction action{ CombatActionType::DEFENSE, -1, -1, -1 };
		EXPECT_ANY_THROW(combat_manager->nextStateByAction(action));

		// need mock to test more
	}

	// CombatManager::generateActionsForPlayer()
	TEST(CombatManager, shouldInitializePlaceUnitsAtStartPosition) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed
		auto& current_state = combat_manager->getCurrentState();
		auto attacker_units = current_state.attacker.getUnits();
		auto defender_units = current_state.defender.getUnits();

		std::for_each(std::begin(attacker_units), std::end(attacker_units), [](const auto _obj) { EXPECT_NE(-1, _obj->getHex()); });
		std::for_each(std::begin(defender_units), std::end(defender_units), [](const auto _obj) { EXPECT_NE(-1, _obj->getHex()); });
	}

	// CombatManager::isUnitMove()
	TEST(CombatManager, shouldReturnTrueIfUnitMoveFalseOtherwise) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(combat_manager->isUnitMove()); // battle didnt start so theres no unit move yet
		combat_manager->nextState(); // start battle
		EXPECT_TRUE(combat_manager->isUnitMove()); // now it is unit move;
		
		CombatAction defend_action{ CombatActionType::DEFENSE, -1, -1, -1 };
		combat_manager->nextStateByAction(defend_action);
		EXPECT_TRUE(combat_manager->isUnitMove()); // first unit in turn defended, but there is another unit in this turn to move
		combat_manager->nextStateByAction(defend_action);
		EXPECT_FALSE(combat_manager->isUnitMove()); // all units in turn moved so there is no unit move now
		combat_manager->nextState(); // start another turn
		EXPECT_TRUE(combat_manager->isUnitMove()); // now it is unit move again
	}

	// CombatManager::isPlayerMove()
	TEST(CombatManager, shouldReturnTrueIfPlayerMoveFalseOtherwise) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(combat_manager->isPlayerMove()); // battle didnt start so theres no player move yet
		combat_manager->nextState(); // start battle
		EXPECT_TRUE(combat_manager->isPlayerMove()); // now it is player move (as he starts first with this configuration);

		CombatAction defend_action{ CombatActionType::DEFENSE, -1, -1, -1 };
		combat_manager->nextStateByAction(defend_action);
		EXPECT_FALSE(combat_manager->isPlayerMove()); // no, there is ai move now
		combat_manager->nextStateByAction(defend_action);
		EXPECT_FALSE(combat_manager->isPlayerMove()); // all units in turn moved so there is battle move now
		combat_manager->nextState(); // start another turn
		EXPECT_TRUE(combat_manager->isPlayerMove()); // now it is player move again
	}

	// CombatManager::isCombatFinished()
	TEST(CombatManager, shouldReturnTrueIfCombatFinishedFalseOtherwise) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 200)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		// createCombatManager calls initialize,  units should be placed

		EXPECT_FALSE(combat_manager->isCombatFinished()); // battle didnt start so cant be finishd
		combat_manager->nextState(); // start battle
		EXPECT_FALSE(combat_manager->isCombatFinished()); // any unit from both sides still alive

		CombatAction defend_action{ CombatActionType::DEFENSE, -1, -1, -1 };
		combat_manager->nextStateByAction(defend_action);
		EXPECT_FALSE(combat_manager->isCombatFinished()); // any unit from both sides still alive
		combat_manager->nextStateByAction(defend_action);
		EXPECT_FALSE(combat_manager->isCombatFinished()); // any unit from both sides still alive
		combat_manager->nextState(); // start another turn
		EXPECT_FALSE(combat_manager->isCombatFinished()); // any unit from both sides still alive
		
		CombatAction kill_action{ CombatActionType::ATTACK, 0, 99, -1 };
		combat_manager->nextStateByAction(kill_action); // player unit attacks ai unit and kill it
		EXPECT_TRUE(combat_manager->isCombatFinished()); // this ends battle because no units left in ai hero
	}
}