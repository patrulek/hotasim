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

	// CombatAI::getReachableHexesForUnit
	TEST(CombatAI, shouldReturnAllHexesInRangPlusAdjacenteWhenNoObstaclesAround) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		auto& ai = combat_manager->getCombatAI();

		// start battle
		combat_manager->nextState();

		auto& active_stack = combat_manager->getActiveStack();
		auto expected = const_cast<CombatPathfinder&>(ai.getPathfinder()).getWalkableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd + 1, combat_manager->getCurrentState().field);
		auto value = ai.getAttackableHexesForUnit(active_stack);
		value.erase(std::find(std::begin(value), std::end(value), active_stack.getHex())); // its not walkable but its attackable
		EXPECT_EQ(expected, value);
	}


	// CombatAI::getAttackableHexesForUnit
	TEST(CombatAI, shouldReturnAllWalkableHexesInRangePlusAdjacentWhenSomeObstaclesAround) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		auto& ai = combat_manager->getCombatAI();

		// start battle
		combat_manager->nextState();

		auto& active_stack = combat_manager->getActiveStack();
		auto& pathfinder = const_cast<CombatPathfinder&>(ai.getPathfinder());
		size_t walkable_size = pathfinder.getWalkableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd + 1, combat_manager->getCurrentState().field).size();
		combat_manager->getCurrentState().field.fillHex(getHexId(5, 2), CombatHexOccupation::SOLID_OBSTACLE);
		pathfinder.clearCache();
		const_cast<CombatAI&>(ai).initializeBattle();


		auto expected = pathfinder.getWalkableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd + 1, combat_manager->getCurrentState().field, true);
		expected.erase(std::find(std::begin(expected), std::end(expected), getHexId(5, 5))); // this hex is not reachable (and attackable) due to obstacle
		auto value = ai.getAttackableHexesForUnit(active_stack);
		EXPECT_EQ(expected, value);
		EXPECT_EQ(walkable_size, value.size() + 1);  // one hexes are not attackable (5,5 - not reachable)
	}

	// CombatAI::getReachableHexesForUnit
	TEST(CombatAI, shouldReturnAllWalkableHexesInRangPlusAdjacenteWhenSomeObstaclesAroundAndEnemyUnitHexes) {
		auto combat_manager = createCombatManager(createHero(createArmy("Peasant", 100)), createHero(createArmy("Peasant", 100), CombatSide::DEFENDER));
		auto& ai = combat_manager->getCombatAI();

		// start battle
		combat_manager->nextState();

		// turn 1
		CombatAction player_action{ CombatActionType::DEFENSE, -1, -1, true };
		combat_manager->nextStateByAction(player_action);
		CombatAction ai_action{ CombatActionType::WALK, 32, getHexId(5, 2), true };
		combat_manager->nextStateByAction(ai_action);
		combat_manager->nextState();

		// turn 2
		auto& active_stack = combat_manager->getActiveStack();
		size_t walkable_size = const_cast<CombatPathfinder&>(ai.getPathfinder()).getWalkableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd + 1, combat_manager->getCurrentState().field, true).size();
		combat_manager->getCurrentState().field.fillHex(getHexId(5, 3), CombatHexOccupation::SOLID_OBSTACLE);
		const_cast<CombatPathfinder&>(ai.getPathfinder()).clearCache();
		const_cast<CombatAI&>(ai).initializeBattle();

		auto expected = const_cast<CombatPathfinder&>(ai.getPathfinder()).getWalkableHexesInRange(active_stack.getHex(), active_stack.getCombatStats().spd + 1, combat_manager->getCurrentState().field, true);
		auto value = ai.getAttackableHexesForUnit(active_stack);
		expected.erase(std::find(std::begin(expected), std::end(expected), getHexId(5, 5))); // this hex is not attackable due to unit at (5, 2)
		EXPECT_EQ(expected, value);
		EXPECT_EQ(walkable_size, value.size() + 2); // two hexes are not attackable (5,5 - not reachable) and (5,3 - not attackable, obstacle)
	}


	// CombatAI::calculateFightValueAdvantageOnHexes(attacker, enemy_hero, field)
	TEST(CombatAI, DISABLED_shouldSetFightValueGainOnHexes) {
		EXPECT_TRUE(false);
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatAI, shouldChooseExactlyThisUnitToAttackIfOnlyOneUnitStackLeftInEnemyHero) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		auto unit = manager->getCurrentState().attacker.getUnits()[0];
		auto& field = manager->getCurrentState().field;

		std::vector<int16_t> hexes{ ai.chooseHexToMoveForAttack(*active_stack, *unit) };
		// only one unit
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker, hexes).size());

		// idx of this unit in enemy hero unit vector
		EXPECT_EQ(0, ai.chooseUnitToAttack(*active_stack, manager->getCurrentState().attacker, hexes).front());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatAI, shouldChooseSecondUnitStackToChaseWhenEqualStacksAndCannotAttackBoth) {
		auto combat_manager = createCombatManager(createHero(createArmy("Imp", 100, "Imp", 100)), createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		auto& ai = combat_manager->getCombatAI();

		// start battle
		combat_manager->nextState();

		// start turn 1
		CombatAction player_action{ CombatActionType::WAIT, -1, -1, true };
		combat_manager->nextStateByAction(player_action);
		player_action = CombatAction{ CombatActionType::WAIT, -1, -1, true };
		combat_manager->nextStateByAction(player_action);

		auto& field = combat_manager->getCurrentState().field;
		field.setTemplate(CombatFieldTemplate::TMP1);
		auto& active_stack = combat_manager->getActiveStack();
		auto& attacker = combat_manager->getCurrentState().attacker;
		auto unit = combat_manager->getCurrentState().attacker.getUnits()[0];
		auto unit2 = combat_manager->getCurrentState().attacker.getUnits()[1];
		std::vector<int16_t> hexes{ ai.chooseHexToMoveForAttack(active_stack, *unit), ai.chooseHexToMoveForAttack(active_stack, *unit2) };

		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).front());

		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 13)); 
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(active_stack, *unit), ai.chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).front());

		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 10));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(active_stack, *unit), ai.chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).front());

		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 7));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(active_stack, *unit), ai.chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).front());

		const_cast<CombatUnit&>(active_stack).moveTo(getHexId(7, 4));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(active_stack, *unit), ai.chooseHexToMoveForAttack(active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(active_stack, attacker, hexes).front());
	}

	// CombatAI::chooseUnitToAttack(attacker, enemy_hero)
	TEST(CombatAI, shouldChooseRandomlyFromTwoStacksIfEqualStacksAndCanAttackBoth) {
		auto manager = createCombatManager();
		auto& ai = manager->getCombatAI();

		auto active_stack = manager->getCurrentState().defender.getUnits()[0];
		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(5, 15));

		manager->getCurrentState().attacker = createHero(createArmy("Imp", 100, "Imp", 100));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(2, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[1])->moveTo(getHexId(8, 1));
		auto& attacker = manager->getCurrentState().attacker;
		auto unit = manager->getCurrentState().attacker.getUnits()[0];
		auto unit2 = manager->getCurrentState().attacker.getUnits()[1];
		std::vector<int16_t> hexes{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };

		auto& field = manager->getCurrentState().field;
		field.setTemplate(CombatFieldTemplate::TMP1);

		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 13));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[0])->moveTo(getHexId(7, 1));
		const_cast<CombatUnit*>(manager->getCurrentState().attacker.getUnits()[1])->moveTo(getHexId(6, 1));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 10));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 7));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).size());
		EXPECT_EQ(1, ai.chooseUnitToAttack(*active_stack, attacker, hexes).front());

		const_cast<CombatUnit*>(active_stack)->moveTo(getHexId(7, 4));
		hexes = std::vector<int16_t>{ ai.chooseHexToMoveForAttack(*active_stack, *unit), ai.chooseHexToMoveForAttack(*active_stack, *unit2) };
		EXPECT_EQ(2, ai.chooseUnitToAttack(*active_stack, attacker, hexes).size());
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
		field.setTemplate(CombatFieldTemplate::TMP1);

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
		field.setTemplate(CombatFieldTemplate::TMP1);
	}
}; // CombatAITest