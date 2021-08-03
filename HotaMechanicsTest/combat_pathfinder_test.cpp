#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_pathfinder.h"
#include "../HotaMechanics/combat_field.h"
#include "utils.h"

namespace CombatPathfinderTest {

	std::vector<int> getCombatFieldTemplate(const int type) {
		// TODO: generate different field templates
		if (type == 1)
			return std::vector<int>{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};


		return std::vector<int>{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
	}

	TEST(CombatPathfinder, shouldReturnCorrectArrayOfAdjacentHexesIdsToGivenHex) {
		auto hex = getHexId(0, 0);
		CombatPathfinder pathfinder;

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

	TEST(CombatPathfinder, shouldReturnCorrectDistanceBetweenHexes) {
		auto hex = getHexId(0, 0);
		auto target_hex = getHexId(0, 0);
		CombatPathfinder pathfinder;

		EXPECT_EQ(0, pathfinder.distanceBetweenHexes(hex, target_hex));

		target_hex = -1;
		EXPECT_EQ(-1, pathfinder.distanceBetweenHexes(hex, target_hex));

		for (auto adjacent_hex : pathfinder.getAdjacentHexes(hex))
			if (adjacent_hex != -1)
				EXPECT_EQ(1, pathfinder.distanceBetweenHexes(hex, adjacent_hex));

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


	TEST(CombatPathfinder, shouldReturnHexesInRangeToGivenHex) {
		CombatPathfinder pathfinder;
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



	TEST(CombatPathfinder, shouldReturnOnlyWalkableHexesInRangeToGivenHexWhenNoObstaclesInRange) {
		CombatPathfinder pathfinder;
		CombatField field;
		int hex = getHexId(2, 1);
		int unit_speed = 6;

		std::vector<int> expected{ 1, 2, 3, 4, 5, 6, 18, 19, 20, 21, 22, 23, 24, 35, 36, 37, 38, 39, 40, 41, 52, 53, 54, 55, 56, 57, 58,
											69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
											137, 138, 139, 140 };
		auto value = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		EXPECT_EQ(expected, value);


		hex = getHexId(5, 15);
		unit_speed = 3;
		expected = std::vector<int>{ 47, 48, 49, 64, 65, 66, 80, 81, 82, 83, 97, 98, 99, 100, 114, 115, 116, 117,
												132, 133, 134, 149, 150, 151 };
		value = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		EXPECT_EQ(expected, value);


		hex = getHexId(8, 1);
		unit_speed = 3;

		expected = std::vector<int>{ 86, 87, 88, 103, 104, 105, 120, 121, 122, 123, 137, 138, 139, 140,
												154, 155, 156, 157, 171, 172, 173 };
		value = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		EXPECT_EQ(expected, value);
	}


	TEST(CombatPathfinder, shouldReturnOnlyWalkableHexesInRangeToGivenHexWhenSomeObstaclesOrUnitsInRange) {
		CombatPathfinder pathfinder;
		CombatField field;
		int hex = getHexId(2, 1);
		int unit_speed = 6;

		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(2, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(3, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);

		std::vector<int> expected{ 1, 2, 3, 4, 5, 6, 18, 20, 21, 22, 23, 24, 35, 36, 38, 39, 40, 41, 52, 53, 55, 56, 57, 58,
											69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
											137, 138, 139, 140 };
		auto value = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		EXPECT_EQ(expected, value);
	}


	TEST(CombatPathfinder, shouldReturnEmptyPathIfTargetHexIsntWalkable) {
		CombatPathfinder pathfinder;
		CombatField field;
		int from = getHexId(8, 1);
		int to = getHexId(8, 16);
		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty());

		field.fillHex(getHexId(5, 5), CombatHexOccupation::UNIT);
		to = getHexId(5, 5);
		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty());

		field.fillHex(getHexId(5, 7), CombatHexOccupation::SOLID_OBSTACLE);
		to = getHexId(5, 7);
		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty());
	}

	TEST(CombatPathfinder, shouldReturnEmptyPathIfTargetHexIsntReachable) {
		CombatPathfinder pathfinder;
		CombatField field;
		int from = getHexId(8, 1);
		int to = getHexId(10, 1);

		field.fillHex(getHexId(9, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(9, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(10, 2), CombatHexOccupation::SOLID_OBSTACLE);

		EXPECT_TRUE(pathfinder.findPath(from, to, field).empty());
	}



	TEST(CombatPathfinder, shouldReturnShortestPathIfTargetIsReachable) {
		CombatPathfinder pathfinder;
		CombatField field;
		int from = getHexId(2, 1);
		int to = getHexId(8, 1);
		EXPECT_EQ(6, pathfinder.findPath(from, to, field).size());

		to = getHexId(2, 15);
		EXPECT_EQ(14, pathfinder.findPath(from, to, field).size());

		to = getHexId(2, 7);
		EXPECT_EQ(6, pathfinder.findPath(from, to, field).size());

		to = getHexId(5, 15);
		EXPECT_EQ(15, pathfinder.findPath(from, to, field).size());

		field.setTemplate(getCombatFieldTemplate(1));
		EXPECT_EQ(15, pathfinder.findPath(from, to, field).size());

		from = getHexId(2, 8);
		to = getHexId(7, 8);
		EXPECT_EQ(6, pathfinder.findPath(from, to, field).size());

		to = getHexId(6, 9);
		EXPECT_EQ(7, pathfinder.findPath(from, to, field).size());
	}

	TEST(CombatPathfinder, shouldReachableHexesEqualWalkableHexesSubstractCurrentPosIfNoObstaclesAroundForSingleHexWalkingUnit) {
		CombatPathfinder pathfinder;
		CombatField field;
		int hex = getHexId(2, 1);
		int unit_speed = 6;

		auto walkable = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex), std::end(walkable));
		auto reachable = pathfinder.getReachableHexesFromWalkableHexes(hex, unit_speed, walkable, false, false, field);
		EXPECT_EQ(walkable, reachable);
	}

	TEST(CombatPathfinder, shouldReturnLessReachableHexesThanWalkableHexesSubstractCurrentPosIfSomeUnitsAndObstaclesAroundForSingleHexWalkingUnit) {
		CombatPathfinder pathfinder;
		CombatField field;
		int hex = getHexId(2, 1);
		int unit_speed = 6;


		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(1, 1), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);
		field.fillHex(getHexId(0, 2), CombatHexOccupation::SOLID_OBSTACLE);

		auto walkable = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex), std::end(walkable));
		auto reachable = pathfinder.getReachableHexesFromWalkableHexes(hex, unit_speed, walkable, false, false, field);

		std::vector<int> expected(walkable);
		std::vector<int> excluded{ getHexId(0, 1) };

		for (auto ex : excluded)
			expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
		EXPECT_EQ(expected, reachable);


		field.clearHex(getHexId(1, 1));
		field.clearHex(getHexId(0, 2));
		field.fillHex(getHexId(2, 2), CombatHexOccupation::SOFT_OBSTACLE);
		field.fillHex(getHexId(2, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(3, 3), CombatHexOccupation::SOLID_OBSTACLE);
		field.fillHex(getHexId(1, 2), CombatHexOccupation::UNIT);


		walkable = pathfinder.getWalkableHexesFromList(pathfinder.getHexesInRange(hex, unit_speed), field);
		walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex), std::end(walkable));
		reachable = pathfinder.getReachableHexesFromWalkableHexes(hex, unit_speed, walkable, false, false, field);

		expected = std::vector<int>(walkable);
		excluded = std::vector<int>{ getHexId(2, 7), getHexId(3, 7) };

		for (auto ex : excluded)
			expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
		EXPECT_EQ(expected, reachable);
	}
}

