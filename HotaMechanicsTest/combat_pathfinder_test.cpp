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
	
	// CombatPathfinder::getUnitStartHex
	TEST(CombatPathfinder, DISABLED_shouldSetCorrectStartPositionsForBattleForUnits) {
		EXPECT_TRUE(false);
	}

	// CombatPathfinder::areAdjacent(_source_hex, _target_hex)
	TEST(CombatPathfinder, shouldReturnTrueIfHexesAdjacentFalseOtherwise) {
		CombatPathfinder pathfinder;

		EXPECT_ANY_THROW(pathfinder.areAdjacent(-1, 150)); // invalid source
		EXPECT_ANY_THROW(pathfinder.areAdjacent(150, -1)); // invalid target

		EXPECT_FALSE(pathfinder.areAdjacent(0, 0)); // source_hex == target_hex
		EXPECT_FALSE(pathfinder.areAdjacent(0, 186)); // not adjacent
		EXPECT_TRUE(pathfinder.areAdjacent(0, 1)); // adjacent
		EXPECT_TRUE(pathfinder.areAdjacent(87, 70)); // adjacent
	}

	// CombatPathfinder::getAdjacentHexes(_source_hex)
	TEST(CombatPathfinder, shouldReturnCorrectArrayOfAdjacentHexesIdsToGivenHex) {
		CombatPathfinder pathfinder;

		EXPECT_ANY_THROW(pathfinder.getAdjacentHexes(-1)); // invalid source

		auto hex = getHexId(0, 0);
		auto expected = std::array<int, 6>{-1, -1, -1, 1, 17, 18};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(0, 16);
		expected = std::array<int, 6>{-1, -1, 15, -1, 33, -1};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(10, 0);
		expected = std::array<int, 6>{153, 154, -1, 171, -1, -1};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(10, 16);
		expected = std::array<int, 6>{169, -1, 185, -1, -1, -1};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(9, 0);
		expected = std::array<int, 6>{-1, 136, -1, 154, -1, 170};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(9, 16);
		expected = std::array<int, 6>{151, 152, 168, -1, 185, 186};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		hex = getHexId(4, 8);
		expected = std::array<int, 6>{59, 60, 75, 77, 93, 94};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));
	}

	// CombatPathfinder::getAdjacentHexesClockwise(); CombatPathfinder::getAdjacentHexesCounterClockwise()
	TEST(CombatPathfinder, shouldReturnCorrectAdjacentHexesClockwiseAndCounterClockwise) {
		CombatPathfinder pathfinder;

		auto hex = getHexId(10, 16);
		auto expected = std::array<int, 6>{169, -1, 185, -1, -1, -1};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexes(hex));

		EXPECT_ANY_THROW(pathfinder.getAdjacentHexesClockwise(-1)); // invalid source hex
		expected = std::array<int, 6>{-1, -1, -1, -1, 185, 169};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexesClockwise(hex));

		EXPECT_ANY_THROW(pathfinder.getAdjacentHexesCounterClockwise(-1)); // invalid source hex
		expected = std::array<int, 6>{169, 185, -1, -1, -1, -1};
		EXPECT_EQ(expected, pathfinder.getAdjacentHexesCounterClockwise(hex));
	}

	// CombatPathfinder::getHexesInRange(_source_hex, _range)
	TEST(CombatPathfinder, shouldReturnHexesInRangeToGivenHex) {
		CombatPathfinder pathfinder;

		EXPECT_ANY_THROW(pathfinder.getHexesInRange(-1, 5)); // invalid source hex
		EXPECT_ANY_THROW(pathfinder.getHexesInRange(150, 0)); // invalid range == 0
		EXPECT_ANY_THROW(pathfinder.getHexesInRange(150, -1)); // invalid range < 0

		int hex = getHexId(2, 1);
		int unit_speed = 6;
		std::vector<int> expected{ 0, 1, 2, 3, 4, 5, 6, 17, 18, 19, 20, 21, 22, 23, 24, 34, 35, 36, 37, 38, 39, 40, 41, 51, 52, 53, 54, 55, 56, 57, 58,
											68, 69, 70, 71, 72, 73, 74, 85, 86, 87, 88, 89, 90, 91, 102, 103, 104, 105, 106, 107, 119, 120, 121, 122, 123, 124,
											136, 137, 138, 139, 140 };
		auto value = pathfinder.getHexesInRange(hex, unit_speed);
		EXPECT_EQ(expected, value);

		hex = getHexId(5, 15);
		unit_speed = 3;
		expected = std::vector<int>{ 47, 48, 49, 50, 64, 65, 66, 67, 80, 81, 82, 83, 84, 97, 98, 99, 100, 101, 114, 115, 116, 117, 118,
												132, 133, 134, 135, 149, 150, 151, 152 };
		value = pathfinder.getHexesInRange(hex, unit_speed);
		EXPECT_EQ(expected, value);


		hex = getHexId(8, 1);
		unit_speed = 3;
		expected = std::vector<int>{ 85, 86, 87, 88, 102, 103, 104, 105, 119, 120, 121, 122, 123, 136, 137, 138, 139, 140,
												153, 154, 155, 156, 157, 170, 171, 172, 173 };
		value = pathfinder.getHexesInRange(hex, unit_speed);
		EXPECT_EQ(expected, value);
	}

	// CombatPathfinder::getWalkableHexesInRange(_source_hex, _range, _field)
	TEST(CombatPathfinder, shouldReturnWalkableHexesInRange) {
		CombatPathfinder pathfinder;
		CombatField field(createField());

		EXPECT_ANY_THROW(pathfinder.getWalkableHexesInRange(-1, 5, field)); // invalid source hex
		EXPECT_ANY_THROW(pathfinder.getWalkableHexesInRange(150, 0, field)); // invalid range == 0
		EXPECT_ANY_THROW(pathfinder.getWalkableHexesInRange(150, -1, field)); // invalid range < 0

		int hex = getHexId(2, 1);
		int unit_speed = 6;
		std::vector<int> expected{ 1, 2, 3, 4, 5, 6, 18, 19, 20, 21, 22, 23, 24, 35, 36, 37, 38, 39, 40, 41, 52, 53, 54, 55, 56, 57, 58,
											69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
											137, 138, 139, 140 };
		auto value = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		EXPECT_EQ(expected, value); // no obstacles in given range

		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(2, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(3, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);

		expected = std::vector<int>{ 1, 2, 3, 4, 5, 6, 18, 20, 21, 22, 23, 24, 35, 36, 38, 39, 40, 41, 52, 53, 55, 56, 57, 58,
											69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
											137, 138, 139, 140 };
		value = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		EXPECT_EQ(expected, value); // filled with some obstacles at field


		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::EMPTY));
		hex = getHexId(5, 15);
		unit_speed = 3;
		expected = std::vector<int>{ 47, 48, 49, 64, 65, 66, 80, 81, 82, 83, 97, 98, 99, 100, 114, 115, 116, 117,
												132, 133, 134, 149, 150, 151 };
		value = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		EXPECT_EQ(expected, value); // no obstacles in range
	}


	// CombatPathfinder::getReachableHexesInRange(source_hex, range, field, can_fly, double_wide) 
	TEST(CombatPathfinder, shouldReachableHexesEqualWalkableHexesWhenNoObstaclesAroundAndLessHexesOtherwise) {
		CombatPathfinder pathfinder;
		CombatField field(createField());

		EXPECT_ANY_THROW(pathfinder.getReachableHexesInRange(-1, 5, field, false, false)); // invalid source hex
		EXPECT_ANY_THROW(pathfinder.getReachableHexesInRange(150, 0, field, false, false)); // invalid range == 0
		EXPECT_ANY_THROW(pathfinder.getReachableHexesInRange(150, -1, field, false, false)); // invalid range < 0
		EXPECT_ANY_THROW(pathfinder.getReachableHexesInRange(150, 5, field, true, false)); // invalid can_fly (not implemented yet)
		EXPECT_ANY_THROW(pathfinder.getReachableHexesInRange(150, 5, field, false, true)); // invalid double_wide (not implemented yet)

		int hex = getHexId(2, 1);
		int unit_speed = 6;

		field.fillHex(hex, CombatHexOccupation::UNIT);
		auto walkable = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		auto reachable = pathfinder.getReachableHexesInRange(hex, unit_speed, field, false, false);
		EXPECT_EQ(walkable, reachable); // when no obstacles or units around, walkable hexes should equal reachable

		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(1, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(0, 2), CombatHexOccupation::SOLID_OBSTACLE);

		walkable = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		reachable = pathfinder.getReachableHexesInRange(hex, unit_speed, field, false, false);

		std::vector<int> expected(walkable);
		std::vector<int> excluded{ getHexId(0, 1) }; // this one field is not reachable anymore, and path to other walkable field is not distorted
		for (auto ex : excluded)
			expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
		EXPECT_EQ(expected, reachable);

		field.clearHex(getHexId(1, 1));
		field.clearHex(getHexId(0, 2));
		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(2, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(3, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);

		walkable = pathfinder.getWalkableHexesInRange(hex, unit_speed, field);
		reachable = pathfinder.getReachableHexesInRange(hex, unit_speed, field, false, false);

		expected = std::vector<int>(walkable);
		excluded = std::vector<int>{ getHexId(2, 7), getHexId(3, 7) }; // these fields are beyond unit range (distorted path by obstacles)
		for (auto ex : excluded)
			expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
		EXPECT_EQ(expected, reachable);
	}

	// CombatPathfinder::distanceBetweenHexes(_source_hex, _target_hex)
	TEST(CombatPathfinder, shouldReturnCorrectDistanceBetweenHexes) {
		CombatPathfinder pathfinder;

		EXPECT_ANY_THROW(pathfinder.distanceBetweenHexes(-1, 150)); // invalid source hex
		EXPECT_ANY_THROW(pathfinder.distanceBetweenHexes(150, -1)); // invalid target hex

		auto hex = getHexId(0, 0);
		auto target_hex = getHexId(0, 0);
		EXPECT_EQ(0, pathfinder.distanceBetweenHexes(hex, target_hex)); // equal hexes

		for (auto adjacent_hex : pathfinder.getAdjacentHexes(hex))
			if (adjacent_hex != -1)
				EXPECT_EQ(1, pathfinder.distanceBetweenHexes(hex, adjacent_hex)); // adjacent hexes

		target_hex = getHexId(0, 15);
		EXPECT_EQ(15, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(10, 0);
		EXPECT_EQ(10, pathfinder.distanceBetweenHexes(hex, target_hex));

		hex = getHexId(2, 1);
		target_hex = getHexId(5, 15);
		EXPECT_EQ(15, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(8, 4);
		EXPECT_EQ(6, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 7);
		EXPECT_EQ(9, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(0, 8);
		EXPECT_EQ(8, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(0, 7);
		EXPECT_EQ(7, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 1);
		EXPECT_EQ(7, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = getHexId(9, 2);
		EXPECT_EQ(7, pathfinder.distanceBetweenHexes(hex, target_hex));

		hex = getHexId(5, 15);
		target_hex = getHexId(2, 13);
		EXPECT_EQ(3, pathfinder.distanceBetweenHexes(hex, target_hex));
	}

	// CombatPathfinder::findPath(source_hex, target_hex, field)
	TEST(CombatPathfinder, shouldReturnEmptyPathIfTargetHexIsntReachable) {
		CombatPathfinder pathfinder;
		CombatField field(createField());

		EXPECT_ANY_THROW(pathfinder.findPath(-1, 150, field)); // invalid source hex
		EXPECT_ANY_THROW(pathfinder.findPath(150, -1, field)); // invalid target hex
		EXPECT_ANY_THROW(pathfinder.findPath(150, 138, field, true)); // finding path for double wide units (not implemented yet)
		EXPECT_TRUE(pathfinder.findPath(5, 5, field).empty()); // equal source and target result in empty path

		int from = getHexId(8, 1);
		int to = getHexId(10, 2);

		field.fillHex(getHexId(9, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(9, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(10, 2), CombatHexOccupation::SOLID_OBSTACLE);

		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty()); // not walkable hex is also not reachable
		to = getHexId(10, 1);
		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty()); // blocked hex is not reachable
	}



	TEST(CombatPathfinder, shouldReturnShortestPathIfTargetIsReachable) {
		CombatPathfinder pathfinder;
		CombatField field(createField());

		int from = getHexId(2, 1);
		int to = getHexId(8, 1);
		auto path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(6, path.size());
		auto expected = std::vector<int>{ 53, 70, 88, 104, 121, to };
		EXPECT_EQ(expected, path);

		to = getHexId(2, 15);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(14, path.size());
		expected = std::vector<int>{ 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, to };
		EXPECT_EQ(expected, path);

		to = getHexId(2, 7);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(6, path.size());
		expected = std::vector<int>{ 36, 37, 38, 39, 40, to };
		EXPECT_EQ(expected, path);

		to = getHexId(5, 15);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(15, path.size());
		expected = std::vector<int>{ 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 65, 82, to };
		EXPECT_EQ(expected, path);

		field.setTemplate(getCombatFieldTemplate(CombatFieldTemplate::IMPS_2x100));
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(15, path.size());
		expected = std::vector<int>{ 36, 37, 38, 39, 40, 41, 42, 43, 61, 78, 79, 80, 81, 82, to };
		EXPECT_EQ(expected, path);

		from = getHexId(2, 8);
		to = getHexId(10, 8);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(9, path.size());
		expected = std::vector<int>{ 60, 76, 93, 109, 126, 142, 160, 177, to };
		EXPECT_EQ(expected, path);

		to = getHexId(6, 9);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(4, path.size());
		expected = std::vector<int>{ 60, 77, 95, to };
		EXPECT_EQ(expected, path);

		from = getHexId(6, 9);
		field.fillHex(getHexId(3, 12), CombatHexOccupation::UNIT);
		to = getHexId(2, 11);
		path = pathfinder.findPath(from, to, field);
		EXPECT_EQ(7, path.size());
		expected = std::vector<int>{ 95, 78, 79, 80, 64, 46, to };
		EXPECT_EQ(expected, path);
	}
}

