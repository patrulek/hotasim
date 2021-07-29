#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_field.h"

int getHexId(int row, int col) {
	return row * CombatHex::COLS + col;
}

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
}