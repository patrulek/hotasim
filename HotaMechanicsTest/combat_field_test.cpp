#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_field.h"
#include "utils.h"

TEST(CombatHex, shouldReturnCorrectArrayOfAdjacentHexesIdsToGivenHex) {
	CombatHex hex;
	hex.id = getHexId(0, 0);

	auto expected = std::array<int, 6>{-1, -1, -1, 1, 17, 18};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(0, 16);
	expected = std::array<int, 6>{-1, -1, 15, -1, 33, -1};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(10, 0);
	expected = std::array<int, 6>{153, 154, -1, 171, -1, -1};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(10, 16);
	expected = std::array<int, 6>{169, -1, 185, -1, -1, -1};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(9, 0);
	expected = std::array<int, 6>{-1, 136, -1, 154, -1, 170};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(9, 16);
	expected = std::array<int, 6>{151, 152, 168, -1, 185, 186};
	EXPECT_EQ(expected, hex.getAdjacentHexes());

	hex.id = getHexId(4, 8);
	expected = std::array<int, 6>{59, 60, 75, 77, 93, 94};
	EXPECT_EQ(expected, hex.getAdjacentHexes());
}

TEST(CombatHex, shouldReturnCorrectDistanceBetweenHexes) {
	CombatHex hex;
	hex.id = getHexId(0, 0);

	CombatHex target_hex;
	target_hex.id = getHexId(0, 0);
	EXPECT_EQ(0, hex.distanceToHex(target_hex.id));

	target_hex.id = -1;
	EXPECT_EQ(-1, hex.distanceToHex(target_hex.id));

	for (auto adjacent_hex : hex.getAdjacentHexes())
		if (adjacent_hex != -1)
			EXPECT_EQ(1, hex.distanceToHex(adjacent_hex));

	target_hex.id = getHexId(0, 15);
	EXPECT_EQ(15, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(10, 0);
	EXPECT_EQ(10, hex.distanceToHex(target_hex.id));

	hex.id = getHexId(2, 1);
	target_hex.id = getHexId(5, 15);
	EXPECT_EQ(15, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(8, 4);
	EXPECT_EQ(6, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(9, 7);
	EXPECT_EQ(9, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(0, 8);
	EXPECT_EQ(8, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(0, 7);
	EXPECT_EQ(7, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(9, 1);
	EXPECT_EQ(7, hex.distanceToHex(target_hex.id));

	target_hex.id = getHexId(9, 2);
	EXPECT_EQ(7, hex.distanceToHex(target_hex.id));

	hex.id = getHexId(5, 15);
	target_hex.id = getHexId(2, 13);
	EXPECT_EQ(3, hex.distanceToHex(target_hex.id));
}

TEST(CombatField, shouldReturnCombatHexWithEqualIdAsParam) {
	CombatField field;

	EXPECT_EQ(0, field.getById(0).id);
	EXPECT_EQ(1, field.getById(1).id);
	EXPECT_EQ(186, field.getById(186).id);
	EXPECT_EQ(100, field.getById(100).id);
	EXPECT_EQ(69, field.getById(69).id);
}

TEST(CombatField, shouldReturnHexesInRangeToGivenHex) {
	CombatField field;
	int hex_id = getHexId(2, 1);
	int unit_speed = 6;

	std::vector<int> expected{ 0, 1, 2, 3, 4, 5, 6, 17, 18, 19, 20, 21, 22, 23, 24, 34, 35, 36, 37, 38, 39, 40, 41, 51, 52, 53, 54, 55, 56, 57, 58,
										68, 69, 70, 71, 72, 73, 74, 85, 86, 87, 88, 89, 90, 91, 102, 103, 104, 105, 106, 107, 119, 120, 121, 122, 123, 124,
										136, 137, 138, 139, 140 };
	auto value = field.getHexesInRange(hex_id, unit_speed);
	EXPECT_EQ(expected, value);

	hex_id = getHexId(5, 15);
	unit_speed = 3;

	expected = std::vector<int>{ 47, 48, 49, 50, 64, 65, 66, 67, 80, 81, 82, 83, 84, 97, 98, 99, 100, 101, 114, 115, 116, 117, 118,
											132, 133, 134, 135, 149, 150, 151, 152};
	value = field.getHexesInRange(hex_id, unit_speed);
	EXPECT_EQ(expected, value);


	hex_id = getHexId(8, 1);
	unit_speed = 3;

	expected = std::vector<int>{ 85, 86, 87, 88, 102, 103, 104, 105, 119, 120, 121, 122, 123, 136, 137, 138, 139, 140, 
											153, 154, 155, 156, 157, 170, 171, 172, 173};
	value = field.getHexesInRange(hex_id, unit_speed);
	EXPECT_EQ(expected, value);
}

TEST(CombatField, shouldReturnOnlyWalkableHexesInRangeToGivenHexWhenNoObstaclesInRange) {
	CombatField field;
	int hex_id = getHexId(2, 1);
	int unit_speed = 6;

	std::vector<int> expected{ 1, 2, 3, 4, 5, 6, 18, 19, 20, 21, 22, 23, 24, 35, 36, 37, 38, 39, 40, 41, 52, 53, 54, 55, 56, 57, 58,
										69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
										137, 138, 139, 140 };
	auto value = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	EXPECT_EQ(expected, value);


	hex_id = getHexId(5, 15);
	unit_speed = 3;
	expected = std::vector<int>{ 47, 48, 49, 64, 65, 66, 80, 81, 82, 83, 97, 98, 99, 100, 114, 115, 116, 117,
											132, 133, 134, 149, 150, 151 };
	value = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	EXPECT_EQ(expected, value);


	hex_id = getHexId(8, 1);
	unit_speed = 3;

	expected = std::vector<int>{ 86, 87, 88, 103, 104, 105, 120, 121, 122, 123, 137, 138, 139, 140,
											154, 155, 156, 157, 171, 172, 173 };
	value = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	EXPECT_EQ(expected, value);
}

TEST(CombatField, shouldReturnOnlyWalkableHexesInRangeToGivenHexWhenSomeObstaclesOrUnitsInRange) {
	CombatField field;
	int hex_id = getHexId(2, 1);
	int unit_speed = 6;

	field.hexes[2][2].occupiedBy = CombatHexOccupation::SOFT_OBSTACLE;
	field.hexes[2][3].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][3].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[1][2].occupiedBy = CombatHexOccupation::UNIT;

	std::vector<int> expected{ 1, 2, 3, 4, 5, 6, 18, 20, 21, 22, 23, 24, 35, 36, 38, 39, 40, 41, 52, 53, 55, 56, 57, 58,
										69, 70, 71, 72, 73, 74, 86, 87, 88, 89, 90, 91, 103, 104, 105, 106, 107, 120, 121, 122, 123, 124,
										137, 138, 139, 140 };
	auto value = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	EXPECT_EQ(expected, value);
}

TEST(CombatField, shouldReturnEmptyPathIfTargetHexIsntWalkable) {
	CombatField field;
	int from = getHexId(8, 1);
	int to = getHexId(8, 16);
	EXPECT_TRUE(field.findPath(from, to).empty());

	field.hexes[5][5].occupiedBy = CombatHexOccupation::UNIT;
	to = getHexId(5, 5);
	EXPECT_TRUE(field.findPath(from, to).empty());

	field.hexes[5][7].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	to = getHexId(5, 7);
	EXPECT_TRUE(field.findPath(from, to).empty());
}

TEST(CombatField, shouldReturnEmptyPathIfTargetHexIsntReachable) {
	CombatField field;
	int from = getHexId(8, 1);
	int to = getHexId(10, 1);
	field.hexes[9][1].occupiedBy = CombatHexOccupation::UNIT;
	field.hexes[9][2].occupiedBy = CombatHexOccupation::UNIT;
	field.hexes[10][2].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;

	EXPECT_TRUE(field.findPath(from, to).empty());
}

TEST(CombatField, shouldReturnShortestPathIfTargetIsReachable) {
	CombatField field;
	int from = getHexId(2, 1);
	int to = getHexId(8, 1);
	EXPECT_EQ(6, field.findPath(from, to).size());

	to = getHexId(2, 15);
	EXPECT_EQ(14, field.findPath(from, to).size());

	to = getHexId(2, 7);
	EXPECT_EQ(6, field.findPath(from, to).size());

	to = getHexId(5, 15);
	EXPECT_EQ(15, field.findPath(from, to).size());

	field.hexes[2][13].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[2][14].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][8].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][9].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][10].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][11].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[5][8].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[5][9].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[5][11].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[5][12].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	EXPECT_EQ(15, field.findPath(from, to).size());

	from = getHexId(2, 8);
	to = getHexId(7, 8);
	EXPECT_EQ(6, field.findPath(from, to).size());

	to = getHexId(6, 9);
	EXPECT_EQ(7, field.findPath(from, to).size());
}

TEST(CombatField, shouldReachableHexesEqualWalkableHexesSubstractCurrentPosIfNoObstaclesAroundForSingleHexWalkingUnit) {
	CombatField field;
	int hex_id = getHexId(2, 1);
	int unit_speed = 6;

	auto walkable = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex_id), std::end(walkable));
	auto reachable = field.getReachableHexesFromWalkableHexes(hex_id, unit_speed, walkable, false, false);
	EXPECT_EQ(walkable, reachable);
}

TEST(CombatField, shouldReturnLessReachableHexesThanWalkableHexesSubstractCurrentPosIfSomeUnitsAndObstaclesAroundForSingleHexWalkingUnit) {
	CombatField field;
	int hex_id = getHexId(2, 1);
	int unit_speed = 6;

	field.hexes[2][2].occupiedBy = CombatHexOccupation::SOFT_OBSTACLE;
	field.hexes[1][1].occupiedBy = CombatHexOccupation::UNIT;
	field.hexes[1][2].occupiedBy = CombatHexOccupation::UNIT;
	field.hexes[0][2].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;

	auto walkable = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex_id), std::end(walkable));
	auto reachable = field.getReachableHexesFromWalkableHexes(hex_id, unit_speed, walkable, false, false);

	std::vector<int> expected(walkable);
	std::vector<int> excluded{ getHexId(0, 1) };

	for( auto ex : excluded)
		expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
	EXPECT_EQ(expected, reachable);


	field.hexes[1][1].occupiedBy = CombatHexOccupation::EMPTY;
	field.hexes[0][2].occupiedBy = CombatHexOccupation::EMPTY;
	field.hexes[2][2].occupiedBy = CombatHexOccupation::SOFT_OBSTACLE;
	field.hexes[2][3].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[3][3].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	field.hexes[1][2].occupiedBy = CombatHexOccupation::UNIT;

	walkable = field.getWalkableHexesFromList(field.getHexesInRange(hex_id, unit_speed));
	walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex_id), std::end(walkable));
	reachable = field.getReachableHexesFromWalkableHexes(hex_id, unit_speed, walkable, false, false);

	expected = std::vector<int>(walkable);
	excluded = std::vector<int>{ getHexId(2, 7), getHexId(3, 7) };

	for (auto ex : excluded)
		expected.erase(std::remove(std::begin(expected), std::end(expected), ex), std::end(expected));
	EXPECT_EQ(expected, reachable);
}