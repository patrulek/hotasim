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

	class CombatManager_GeneratorTest : public ::testing::Test {
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

	// CombatManager::generateActionsForPlayer()
	TEST_F(CombatManager_GeneratorTest, shouldReturnNoAttackNorSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndNoHostileUnitsInRange) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		auto actions = manager->generateActionsForPlayer();
		EXPECT_EQ(21, actions.size()); // 19 walking actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		for (auto& action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(19, walking_actions);

		CombatAction player_action{ CombatActionType::DEFENSE, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(8, 15), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		ai_action = CombatAction{ CombatActionType::DEFENSE, -1, 0xFF, true };
		manager->nextStateByAction(ai_action);

		actions = manager->generateActionsForPlayer(); 
		EXPECT_EQ(20, actions.size()); // 19 walking actions, 1 defend action

		walking_actions = 0;
		for (auto& action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(19, walking_actions);

		/// --- ///
		// TODO: need FIELD_CHANGE event
		//current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::SOLID_OBSTACLE); // this obstacle prevents from going to 2 hexes, so there should be 20 actions now

		//actions = manager->generateActionsForPlayer();
		//EXPECT_EQ(19, actions.size()); // 17 walking actions, 1 wait action, 1 defend action

		//walking_actions = 0;
		//for (auto action : actions) {
		//	EXPECT_FALSE(isAttackOrSpellcastAction(action));
		//	walking_actions += (action.action == CombatActionType::WALK);
		//}
		//EXPECT_EQ(17, walking_actions);

		///// --- ///

		
	}

	// CombatManager::generateActionsForPlayer()
	TEST_F(CombatManager_GeneratorTest, shouldReturnNoSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndHostileUnitsInRange) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::DEFENSE, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(5, 2), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		auto actions = manager->generateActionsForPlayer();
		EXPECT_EQ(25, actions.size()); // 17 walking actions ([5,2] blokuje), 6 attack actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		int attack_actions = 0;
		for (auto action : actions) {
			walking_actions += (action.action == CombatActionType::WALK);
			attack_actions += (action.action == CombatActionType::ATTACK);
		}
		EXPECT_EQ(17, walking_actions);
		EXPECT_EQ(6, attack_actions);
	}


	// CombatManager::generateActionsForPlayer()
	TEST_F(CombatManager_GeneratorTest, shouldNotGenerateAttackActionsForFriendlyUnits) {
		SetUp(createHero(createArmy("Imp", 50, "Imp", 50)), createHero(createArmy("Peasant", 250), CombatSide::DEFENDER));
		manager->setAllUnitStacks();
		// start battle
		manager->nextState();

		// start turn 1
		auto actions = manager->generateActionsForPlayer();
		int attack_actions = 0;
		for (auto& action : actions) {
			attack_actions += (action.action == CombatActionType::ATTACK);
		}
		EXPECT_EQ(0, attack_actions);
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldReturnAttackActionWhenOneHostileUnitInRange) {
		SetUp(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		
		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(8, 1), true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(8, 2), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, -1 };
		manager->nextStateByAction(player_action);

		// when one player unit in range, always choose that one to attack
		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::ATTACK, actions[0].action);
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, DISABLED_shouldReturnAttackActionWhenManyHostilesUnitsInRange) {
		SetUp(createHero(createArmy("Peasant", 100, "Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(7, 2), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::DEFENSE, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(8, 2), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		// when two player's units in range (at same distance), ane both are same(or very similar) strength, then AI attacks random stack, so generate 2 attack actions
		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(2, actions.size());
		EXPECT_NE(actions[0].param1, actions[1].param1); // actions with different unit_id (targets)

		auto& current_state = manager->getCurrentState();
		const_cast<CombatUnit*>(current_state.attacker.getUnitsPtrs()[0])->applyDamage(50);

		// when two player's units in range, and one of then is definitely weaker, pick the one, that fight value gain is better
		// (100 peasants attacking 100 peasants, will get avg 1500 fight value; 100 peasants attacking 50 peasants, will get avg 750 fight value)
		// in this case AI should choose to attack 100 peasants so unit_id == 1
		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(1, actions[0].param1); // unit_id == 1 (pick unit with greater fv gain)


		//const_cast<CombatUnit*>(current_state.attacker.getUnitsPtrs()[1])->moveTo(getHexId(7, 1));
		//current_state.field.clearHex(getHexId(7, 1));
		//current_state.field.fillHex(getHexId(7, 1), CombatHexOccupation::UNIT);

		// when two player's units in range, ane both are same(or very similar) strength, but at different distance, check closer one
		// TODO: need to confirm this
		//actions = manager->generateActionsForAI();
		//EXPECT_EQ(1, actions.size());
		//EXPECT_EQ(0, actions[0].param1); // unit_id == 0 (pick first unit)
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldReturnWalkActionWhenNoHostileUnitsInRangeAndWeakerUnitsFurtherThanTwoTurnsAway) {
		SetUp(createHero(createArmy("Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(8, 1), true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(8, 15), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, -1 };
		manager->nextStateByAction(player_action);

		// when no player unit in range, choos to walk
		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(8, 2), actions[0].target);
		EXPECT_EQ(3, actions[0].param1); // walk distance = unit_speed
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldReturnWaitActionWhenNoHostileUnitsInRangeAndEqualCloserThanTwoTurnsAwayToStrongerUnitsAndMovingWouldGetUsInDangerZone) {
		SetUp(createHero(createArmy("Imp", 200)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));

		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(8, 1), true };
		manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(8, 8), true };
		manager->nextStateByAction(ai_action);
		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, -1 };
		manager->nextStateByAction(player_action);

		// when one player unit in range, always choose that one to attack
		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldFixFirstWaitActionBug) {
		SetUp(createHero(createArmy("Imp", 50, "Imp", 100, "Imp", 50)), createHero(createArmy("Peasant", 250, "Peasant", 250), CombatSide::DEFENDER));

		manager->getCurrentState().field.setTemplate(CombatFieldTemplate::IMPS_2x100);
		manager->setAllUnitStacks();
		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(5, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 5), true };
		manager->nextStateByAction(player_action);

		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(5, 14), true };
		manager->nextStateByAction(ai_action);
		ai_action = CombatAction{ CombatActionType::WALK, 32, getHexId(8, 12), true };
		manager->nextStateByAction(ai_action);

		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(5, 7), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 7), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(5, 8), actions[0].target);
		EXPECT_EQ(3, actions[0].param1); // result hex (5, 11)
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(6, 7), actions[0].target); 
		EXPECT_EQ(3, actions[0].param1); // result hex (6, 10)
		manager->nextStateByAction(actions[0]);
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldFixFirstWalkAfterWaitActionBug) {
		SetUp(createHero(createArmy("Imp", 50, "Imp", 100, "Imp", 50)), createHero(createArmy("Peasant", 250, "Peasant", 250), CombatSide::DEFENDER));
		manager->getCurrentState().field.setTemplate(CombatFieldTemplate::IMPS_2x100);
		manager->setAllUnitStacks();
		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(1, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(5, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 6), true };
		manager->nextStateByAction(player_action);

		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(5, 14), true };
		manager->nextStateByAction(ai_action);
		ai_action = CombatAction{ CombatActionType::WALK, 32, getHexId(8, 12), true };
		manager->nextStateByAction(ai_action);

		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(5, 9), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(5, 10), actions[0].target);
		EXPECT_EQ(2, actions[0].param1); // result hex (5, 12)
		manager->nextStateByAction(actions[0]);

		actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(5, 10), actions[0].target);
		EXPECT_EQ(1, actions[0].param1); // result hex (8, 11)
		manager->nextStateByAction(actions[0]);
	}


	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldFixSecondWalkAfterWaitActionBug) {
		SetUp(createHero(createArmy("Imp", 50, "Imp", 100, "Imp", 50)), createHero(createArmy("Peasant", 250, "Peasant", 250), CombatSide::DEFENDER));
		manager->getCurrentState().field.setTemplate(CombatFieldTemplate::IMPS_2x100);
		manager->setAllUnitStacks();
		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(1, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(2, 12), true };
		manager->nextStateByAction(ai_action);
		ai_action = CombatAction{ CombatActionType::WALK, 32, getHexId(8, 12), true };
		manager->nextStateByAction(ai_action);

		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(5, 6), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(7, 6), true };
		manager->nextStateByAction(player_action);

		manager->nextState();

		// start turn 2
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(player_action);

		ai_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(ai_action);
		ai_action = CombatAction{ CombatActionType::WAIT, -1, 0xFF, true };
		manager->nextStateByAction(ai_action);
		ai_action = CombatAction{ CombatActionType::WALK, 32, getHexId(5, 11), true };
		manager->nextStateByAction(ai_action);

		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(6, 6), actions[0].target);
		EXPECT_EQ(3, actions[0].param1); // result hex (6, 10)
		manager->nextStateByAction(actions[0]);
	}

	// CombatManager::generateActionsForAI()
	TEST_F(CombatManager_GeneratorTest, shouldFixWrongUnitChooseToAttackBug) {
		SetUp(createHero(createArmy("Imp", 1, "Imp", 120)), createHero(createArmy("Peasant", 250, "Peasant", 250), CombatSide::DEFENDER));
		manager->getCurrentState().field.setTemplate(CombatFieldTemplate::IMPS_2x100);
		manager->setAllUnitStacks();
		// start battle
		manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WALK, 32, getHexId(4, 3), true };
		manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WALK, 32, getHexId(9, 1), true };
		manager->nextStateByAction(player_action);

		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(4, 13), true };
		manager->nextStateByAction(ai_action);

		auto actions = manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(3, 4), actions[0].target);
		EXPECT_EQ(3, actions[0].param1); // result hex (8, 12)
		manager->nextStateByAction(actions[0]);
		EXPECT_EQ(getHexId(8, 12), manager->getCurrentState().defender.getUnitsPtrs()[1]->getHex());
	}
}