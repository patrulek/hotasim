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

	// CombatManager::generateActionsForPlayer()
	TEST(CombatManager_Generator, shouldReturnNoAttackNorSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndNoHostileUnitsInRange) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		combat_manager->nextState();
		auto& current_state = combat_manager->getCurrentState();
		
		auto actions = combat_manager->generateActionsForPlayer();
		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		unit->moveTo(getHexId(8, 1));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		EXPECT_EQ(21, actions.size()); // 19 walking actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(19, walking_actions);

		/// --- ///

		current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::SOLID_OBSTACLE); // this obstacle prevents from going to 2 hexes, so there should be 20 actions now

		actions = combat_manager->generateActionsForPlayer();
		EXPECT_EQ(19, actions.size()); // 17 walking actions, 1 wait action, 1 defend action

		walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(17, walking_actions);

		/// --- ///

		unit->wait(); // were changing unit state, so now wait action should be gone
		actions = combat_manager->generateActionsForPlayer();
		EXPECT_EQ(18, actions.size()); // 17 walking actions, 1 defend action

		walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(17, walking_actions);
	}

	// CombatManager::generateActionsForPlayer()
	TEST(CombatManager_Generator, shouldReturnNoSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndHostileUnitsInRange) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 1), CombatSide::DEFENDER));
		combat_manager->nextState();
		auto& current_state = combat_manager->getCurrentState();

		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 2));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::UNIT); // this unit prevents from going to 2 hexes

		auto actions = combat_manager->generateActionsForPlayer();
		EXPECT_EQ(26, actions.size()); // 18 walking actions, 6 attack actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		int attack_actions = 0;
		for (auto action : actions) {
			walking_actions += (action.action == CombatActionType::WALK);
			attack_actions += (action.action == CombatActionType::ATTACK);
		}
		EXPECT_EQ(18, walking_actions);
		EXPECT_EQ(6, attack_actions);
	}

	// CombatManager::generateActionsForAI()
	TEST(CombatManager_Generator, shouldReturnAttackActionWhenHostileUnitsInRange) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		combat_manager->nextState();
		CombatAction player_action{ CombatActionType::WAIT, -1, -1, -1 };
		combat_manager->nextStateByAction(player_action);
		auto& current_state = combat_manager->getCurrentState();

		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 2));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::UNIT);

		// when one player unit in range, always choose that one to attack
		auto actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::ATTACK, actions[0].action);


		current_state.attacker = createHero(createArmy("Peasant", 100, "Peasant", 100)); 
		const_cast<CombatUnit*>(current_state.attacker.getUnits()[0])->moveTo(getHexId(8, 1));
		const_cast<CombatUnit*>(current_state.attacker.getUnits()[1])->moveTo(getHexId(7, 2));
		current_state.field.fillHex(getHexId(7, 2), CombatHexOccupation::UNIT);

		// when two player's units in range (at same distance), ane both are same(or very similar) strength, then AI attacks random stack, so generate 2 attack actions
		actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(2, actions.size());
		EXPECT_NE(actions[0].param1, actions[1].param1); // actions with different unit_id (targets)


		//const_cast<CombatUnit*>(current_state.attacker.getUnits()[1])->moveTo(getHexId(7, 1));
		//current_state.field.clearHex(getHexId(7, 1));
		//current_state.field.fillHex(getHexId(7, 1), CombatHexOccupation::UNIT);

		// when two player's units in range, ane both are same(or very similar) strength, but at different distance, check closer one
		// TODO: need to confirm this
		//actions = combat_manager->generateActionsForAI();
		//EXPECT_EQ(1, actions.size());
		//EXPECT_EQ(0, actions[0].param1); // unit_id == 0 (pick first unit)


		const_cast<CombatUnit*>(current_state.attacker.getUnits()[0])->applyDamage(50);

		// when two player's units in range, and one of then is definitely weaker, pick the one, that fight value gain is better
		// (100 peasants attacking 100 peasants, will get avg 1500 fight value; 100 peasants attacking 50 peasants, will get avg 750 fight value)
		// in this case AI should choose to attack 100 peasants so unit_id == 1
		actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(1, actions[0].param1); // unit_id == 1 (pick unit with greater fv gain)
	}

	// CombatManager::generateActionsForAI()
	TEST(CombatManager_Generator, shouldReturnWalkActionWhenNoHostileUnitsInRangeAndWeakerUnitsFurtherThanTwoTurnsAway) {
		auto combat_manager = createCombatManager(createHero(createArmy("Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		combat_manager->nextState();
		CombatAction player_action{ CombatActionType::WAIT, -1, -1, -1 };
		combat_manager->nextStateByAction(player_action);
		auto& current_state = combat_manager->getCurrentState();

		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 15));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		current_state.field.fillHex(getHexId(8, 15), CombatHexOccupation::UNIT); 

		// when one player unit in range, always choose that one to attack
		auto actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(8, 2), actions[0].target);
		EXPECT_EQ(3, actions[0].param1); // walk distance = unit_speed
	}



	// CombatManager::generateActionsForAI()
	TEST(CombatManager_Generator, shouldReturnWalkActionWhenNoHostileUnitsInRangeAndEqualCloserThanTwoTurnsAwayToStrongerUnits) {
		auto combat_manager = createCombatManager(createHero(createArmy("Imp", 200)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		combat_manager->nextState();
		CombatAction player_action{ CombatActionType::WAIT, -1, -1, -1 };
		combat_manager->nextStateByAction(player_action);
		auto& current_state = combat_manager->getCurrentState();

		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 9));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		current_state.field.fillHex(getHexId(8, 9), CombatHexOccupation::UNIT); 

		// when one player unit in range, always choose that one to attack
		auto actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WALK, actions[0].action);
		EXPECT_EQ(getHexId(8, 2), actions[0].target);
		EXPECT_EQ(1, actions[0].param1); // walk distance = 1 (because from this point we need only 2 turns to get to the unit (from current pos we need 3), and to this hex, enemy unit cant reach next turn)
	}


	// CombatManager::generateActionsForAI()
	TEST(CombatManager_Generator, shouldReturnWaitActionWhenNoHostileUnitsInRangeAndEqualCloserThanTwoTurnsAwayToStrongerUnitsAndMovingWouldGetUsInDangerZone) {
		auto combat_manager = createCombatManager(createHero(createArmy("Imp", 200)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		combat_manager->nextState();
		CombatAction player_action{ CombatActionType::WAIT, -1, -1, -1 };
		combat_manager->nextStateByAction(player_action);
		auto& current_state = combat_manager->getCurrentState();

		auto unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		auto unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 8));
		current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);
		current_state.field.fillHex(getHexId(8, 8), CombatHexOccupation::UNIT); 

		// when one player unit in range, always choose that one to attack
		auto actions = combat_manager->generateActionsForAI();
		EXPECT_EQ(1, actions.size());
		EXPECT_EQ(CombatActionType::WAIT, actions[0].action);
	}
}