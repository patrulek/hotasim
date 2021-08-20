#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/utils.h"
#include "../HotaMechanics/combat_pathfinder.h"
#include "../HotaMechanics/combat_field.h"
#include "utils.h"

namespace CombatPathfinderTest {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Utils;
	using namespace TestUtils;

	class CombatPathfinderTest : public ::testing::Test {

	protected:
		void SetUp() override {
			pathfinder = std::make_shared<CombatPathfinder>();
		}

		std::shared_ptr<CombatPathfinder> pathfinder;
	};
	
	// CombatPathfinder::getUnitStartHex
	TEST_F(CombatPathfinderTest, DISABLED_shouldSetCorrectStartPositionsForBattleForUnits) {
		EXPECT_TRUE(false);
	}

	// CombatPathfinder::areAdjacent(_source_hex, _target_hex)
	TEST_F(CombatPathfinderTest, shouldReturnTrueIfHexesAdjacentFalseOtherwise) {
		EXPECT_FALSE(pathfinder->areAdjacent(INVALID_HEX_ID, 150)); // invalid source
		EXPECT_FALSE(pathfinder->areAdjacent(150, INVALID_HEX_ID)); // invalid target

		EXPECT_FALSE(pathfinder->areAdjacent(0, 0)); // source_hex == target_hex
		EXPECT_FALSE(pathfinder->areAdjacent(0, 186)); // not adjacent
		EXPECT_TRUE(pathfinder->areAdjacent(0, 1)); // adjacent
		EXPECT_TRUE(pathfinder->areAdjacent(87, 70)); // adjacent
	}

	// CombatPathfinder::getAdjacentHexes(_source_hex)
	TEST_F(CombatPathfinderTest, shouldReturnCorrectArrayOfAdjacentHexesIdsToGivenHex) {
		auto expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(INVALID_HEX_ID)); // invalid source

		HexId hex = getHexId(0, 0);
		expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, 1, 17, 18};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(0, 16);
		expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, 15, INVALID_HEX_ID, 33, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(10, 0);
		expected = AdjacentArray{153, 154, INVALID_HEX_ID, 171, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(10, 16);
		expected = AdjacentArray{169, INVALID_HEX_ID, 185, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(9, 0);
		expected = AdjacentArray{INVALID_HEX_ID, 136, INVALID_HEX_ID, 154, INVALID_HEX_ID, 170};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(9, 16);
		expected = AdjacentArray{151, 152, 168, INVALID_HEX_ID, 185, 186};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		hex = getHexId(4, 8);
		expected = AdjacentArray{59, 60, 75, 77, 93, 94};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));
	}

	// CombatPathfinder::getAdjacentHexesClockwise(); CombatPathfinder::getAdjacentHexesCounterClockwise()
	TEST_F(CombatPathfinderTest, shouldReturnCorrectAdjacentHexesClockwiseAndCounterClockwise) {
		HexId hex = getHexId(10, 16);
		auto expected = AdjacentArray{169, INVALID_HEX_ID, 185, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexes(hex));

		expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexesClockwise(INVALID_HEX_ID)); // invalid source hex

		expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, 185, 169};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexesClockwise(hex));

		expected = AdjacentArray{INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexesCounterClockwise(INVALID_HEX_ID)); // invalid source hex

		expected = AdjacentArray{169, 185, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID};
		EXPECT_EQ(expected, pathfinder->getAdjacentHexesCounterClockwise(hex));
	}

	// CombatPathfinder::getReachableHexesInRange(source_hex, range, field, can_fly, double_wide) 
	TEST_F(CombatPathfinderTest, DISABLED_shouldReachableHexesEqualWalkableHexesWhenNoObstaclesAroundAndLessHexesOtherwise) {
		CombatField field(createField());

		EXPECT_TRUE(pathfinder->getReachableHexesInRange(INVALID_HEX_ID, 5, field, false, false).empty()); // invalid source hex
		EXPECT_TRUE(pathfinder->getReachableHexesInRange(150, 0, field, false, false).empty()); // invalid range == 0
		EXPECT_TRUE(pathfinder->getReachableHexesInRange(150, -1, field, false, false).empty()); // invalid range < 0
		EXPECT_ANY_THROW(pathfinder->getReachableHexesInRange(150, 5, field, true, false)); // invalid can_fly (not implemented yet)
		EXPECT_ANY_THROW(pathfinder->getReachableHexesInRange(150, 5, field, false, true)); // invalid double_wide (not implemented yet)

		HexId hex = getHexId(2, 1);
		uint8_t unit_speed = 6;

		field.fillHex(hex, CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		auto& reachable = pathfinder->getReachableHexesInRange(hex, unit_speed, field, false, false);
		auto expected = std::vector<HexId>{
			// TODO;
		};
		EXPECT_EQ(expected, reachable); // when no obstacles or units around, walkable hexes should equal reachable

		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(1, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(0, 2), CombatHexOccupation::SOLID_OBSTACLE);
		pathfinder->clearPathCache();

		expected = std::vector<HexId>{
			// TODO;
		};
		reachable = pathfinder->getReachableHexesInRange(hex, unit_speed, field, false, false);
		EXPECT_EQ(expected, reachable);

		field.clearHex(getHexId(1, 1));
		field.clearHex(getHexId(0, 2));
		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(2, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(3, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);
		pathfinder->clearPathCache();
		
		expected = std::vector<HexId>{
			// TODO;
		};
		reachable = pathfinder->getReachableHexesInRange(hex, unit_speed, field, false, false);
		EXPECT_EQ(expected, reachable);
	}

	// CombatPathfinder::distanceBetweenHexes(_source_hex, _target_hex)
	TEST_F(CombatPathfinderTest, shouldReturnCorrectDistanceBetweenHexes) {
		EXPECT_EQ(MAX_FIELD_RANGE, pathfinder->distanceBetweenHexes(150, INVALID_HEX_ID)); // invalid target hex
		EXPECT_EQ(MAX_FIELD_RANGE, pathfinder->distanceBetweenHexes(INVALID_HEX_ID, 150)); // invalid source hex

		HexId hex = getHexId(0, 0);
		HexId target_hex = getHexId(0, 0);
		EXPECT_EQ(0, pathfinder->distanceBetweenHexes(hex, target_hex)); // equal hexes

		target_hex = getHexId(0, 15);
		EXPECT_EQ(MAX_FIELD_RANGE, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(10, 0);
		EXPECT_EQ(MAX_FIELD_RANGE, pathfinder->distanceBetweenHexes(hex, target_hex));

		hex = getHexId(5, 10);
		for (auto adjacent_hex : pathfinder->getAdjacentHexes(hex))
			EXPECT_EQ(1, pathfinder->distanceBetweenHexes(hex, adjacent_hex));

		hex = getHexId(2, 1);
		target_hex = getHexId(5, 15);
		EXPECT_EQ(15, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(8, 4);
		EXPECT_EQ(6, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 7);
		EXPECT_EQ(9, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(0, 8);
		EXPECT_EQ(8, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(0, 7);
		EXPECT_EQ(7, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 1);
		EXPECT_EQ(7, pathfinder->distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 2);
		EXPECT_EQ(7, pathfinder->distanceBetweenHexes(hex, target_hex));

		hex = getHexId(5, 15);
		target_hex = getHexId(2, 13);
		EXPECT_EQ(3, pathfinder->distanceBetweenHexes(hex, target_hex));
	}

	// CombatPathfinder::findPath(source_hex, target_hex, field)
	TEST_F(CombatPathfinderTest, shouldReturnEmptyPathIfTargetHexIsntReachable) {
		CombatField field(createField());

		EXPECT_ANY_THROW(pathfinder->findPath(150, 138, field, true)); // finding path for double wide units (not implemented yet)
		EXPECT_TRUE(pathfinder->findPath(5, 5, field).empty()); // equal source and target result in empty path
		EXPECT_TRUE(pathfinder->findPath(INVALID_HEX_ID, 150, field).empty()); // invalid source hex
		EXPECT_TRUE(pathfinder->findPath(150, INVALID_HEX_ID, field).empty()); // invalid target hex

		HexId from = getHexId(8, 1);
		HexId to = getHexId(10, 2);

		field.fillHex(getHexId(9, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(9, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(10, 2), CombatHexOccupation::SOLID_OBSTACLE);

		EXPECT_TRUE(pathfinder->findPath(from, to, field).empty()); // not walkable hex is also not reachable
		to = getHexId(10, 1);
		EXPECT_TRUE(pathfinder->findPath(from, to, field).empty()); // blocked hex is not reachable
	}

	TEST_F(CombatPathfinderTest, shouldReturnShortestPathIfTargetIsReachable) {
		CombatField field(createField());

		HexId from = getHexId(2, 1);
		HexId to = getHexId(8, 1);
		auto& path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(6, path.size());
		auto expected = std::vector<uint8_t>{ 53, 70, 88, 104, 121, to };
		EXPECT_EQ(expected, path);

		to = getHexId(2, 15);
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(14, path.size());
		expected = std::vector<uint8_t>{ 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, to };
		EXPECT_EQ(expected, path);

		to = getHexId(2, 7);
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(6, path.size());
		expected = std::vector<uint8_t>{ 36, 37, 38, 39, 40, to };
		EXPECT_EQ(expected, path);

		to = getHexId(5, 15);
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(15, path.size());
		expected = std::vector<uint8_t>{ 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 65, 82, to };
		EXPECT_EQ(expected, path);

		field.setTemplate(CombatFieldTemplate::IMPS_2x100);
		field.rehash();
		pathfinder->clearPathCache();
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(15, path.size());
		expected = std::vector<uint8_t>{ 36, 37, 38, 39, 40, 41, 42, 43, 61, 78, 79, 80, 81, 82, to };
		EXPECT_EQ(expected, path);

		from = getHexId(2, 8);
		to = getHexId(10, 8);
		pathfinder->clearPathCache();
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(9, path.size());
		expected = std::vector<uint8_t>{ 60, 76, 93, 109, 126, 142, 160, 177, to };
		EXPECT_EQ(expected, path);

		to = getHexId(6, 9);
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(4, path.size());
		expected = std::vector<uint8_t>{ 60, 77, 95, to };
		EXPECT_EQ(expected, path);

		from = getHexId(6, 9);
		field.fillHex(getHexId(3, 12), CombatHexOccupation::UNIT);
		to = getHexId(2, 11);
		pathfinder->clearPathCache();
		path = pathfinder->findPath(from, to, field);
		EXPECT_EQ(7, path.size());
		expected = std::vector<uint8_t>{ 95, 78, 79, 80, 64, 46, to };
		EXPECT_EQ(expected, path);
	}
}

