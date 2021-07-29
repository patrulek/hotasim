#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>

enum class CombatHexOccupation {
	EMPTY, UNIT, OBSTACLE // etc
};

struct CombatHex {
	static const int COLS = 17;
	static const int ROWS = 11;

	int id;
	CombatHexOccupation occupiedBy;

	int distanceToHex(int target_hex_id) {
		if (target_hex_id == id)
			return 0;

		if (target_hex_id == -1)
			return -1;

		if (isAdjacentHex(target_hex_id))
			return 1;

		bool same_row = id / COLS == target_hex_id / COLS;
		if (same_row) 
			return abs(target_hex_id - id);

		bool same_col = id % COLS == target_hex_id % COLS;
		if (same_col) 
			return abs((target_hex_id - id) / COLS);
		
		int row_dist = abs(id % COLS - target_hex_id % COLS);
		int col_dist = abs(id / COLS - target_hex_id / COLS);

		return row_dist + col_dist - ceil(col_dist / 2.0);
	}

	bool isAdjacentHex(int target_hex_id) {
		auto adjacent_hexes = getAdjacentHexes();
		return std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), target_hex_id) != std::end(adjacent_hexes);
	}

	std::array<int, 6> getAdjacentHexes() {
		std::array<int, 6> hexes{ -1, -1, -1, -1, -1, -1 };
		int offset = -1 * (id / COLS % 2 != 0);

		bool first_row = id < COLS;
		bool last_row = id > COLS * (ROWS - 1) - 1;
		bool first_hex_in_row = id % COLS == 0;
		bool last_hex_in_row = id % COLS == COLS - 1;
		bool even_row = id / COLS % 2 == 1;

		hexes[0] = first_row || (first_hex_in_row && even_row) ? -1 : id - COLS + offset;
		hexes[1] = first_row || (last_hex_in_row && !even_row) ? -1 : id - COLS + offset + 1;
		hexes[2] = first_hex_in_row ? -1 : id - 1;
		hexes[3] = last_hex_in_row ? -1 : id + 1;
		hexes[4] = last_row || (first_hex_in_row && even_row) ? -1 : id + COLS + offset;
		hexes[5] = last_row || (last_hex_in_row && !even_row) ? -1 : id + COLS + offset + 1;

		return hexes;
	}

	CombatHex() = default;
};

enum class CombatFieldType {
	GRASS, DIRT // ...
};

enum class CombatFieldTemplate {
	TMP1, TMP2 // ...
};

class CombatField {
public:
	CombatFieldType combatFieldId{ CombatFieldType::GRASS };
	CombatFieldTemplate combatFieldTemplate{ CombatFieldTemplate::TMP1 };
	CombatHex hexes[11][17];

	CombatField() {
		for (int y = 0; y < 11; ++y)
			for (int x = 0; x < 17; ++x) {
				hexes[y][x].id = y * 17 + x;
				hexes[y][x].occupiedBy = CombatHexOccupation::EMPTY;
			}

		if (combatFieldTemplate == CombatFieldTemplate::TMP1)
			hexes[0][5].occupiedBy = CombatHexOccupation::OBSTACLE;
	}
};