#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <iostream>

enum class CombatHexOccupation {
	EMPTY, UNIT, SOFT_OBSTACLE, SOLID_OBSTACLE // etc
};

class CombatHex {
public:
	static const int COLS = 17;
	static const int ROWS = 11;

	int id;
	CombatHexOccupation occupiedBy;

	void occupyHex(CombatHexOccupation type) {
		occupiedBy = type;
	}

	bool isWalkable() const {
		bool first_col = id % COLS == 0;
		bool last_col = id % COLS == COLS - 1;

		return !(occupiedBy == CombatHexOccupation::SOLID_OBSTACLE || occupiedBy == CombatHexOccupation::UNIT)
			&& !(first_col || last_col);
	}

	int distanceToHex(int target_hex_id) const {
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
		int odd_rows = 0, even_rows = 0;

		for (int y = (id / COLS) + 1; y <= target_hex_id / COLS; ++y) {
			odd_rows += (y % 2 == 1);
			even_rows += (y % 2 == 0);
		}
		for (int y = (id / COLS) - 1; y >= target_hex_id / COLS; --y) {
			odd_rows += (y % 2 == 1);
			even_rows += (y % 2 == 0);
		}

		return row_dist + col_dist - (row_dist >= std::max(odd_rows, even_rows) ? std::max(odd_rows, even_rows) : row_dist);
	}

	bool isAdjacentHex(int target_hex_id) const {
		auto adjacent_hexes = getAdjacentHexes();
		return std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), target_hex_id) != std::end(adjacent_hexes);
	}

	std::array<int, 6> getAdjacentHexesClockwise() const {
		auto hexes = getAdjacentHexes();

		return std::array<int, 6>{ hexes[1], hexes[3], hexes[5], hexes[4], hexes[2], hexes[0] };
	}


	std::array<int, 6> getAdjacentHexes() const {
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

	CombatField(const CombatFieldType _field_type)
			: combatFieldId(_field_type) {
		
		initializeCombatHexes();
		//if (combatFieldTemplate == CombatFieldTemplate::TMP1)
		//	hexes[0][5].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	}

	CombatField()
		: CombatField(CombatFieldType::GRASS) {}

	void initializeCombatHexes() {
		for (int y = 0; y < 11; ++y) {
			for (int x = 0; x < 17; ++x) {
				hexes[y][x].id = y * 17 + x;
				hexes[y][x].occupiedBy = CombatHexOccupation::EMPTY;
			}
		}
	}
		
	void setTemplate(std::vector<int>& _template) {
		for (auto hex : _template) {
			int y = hex / 17;
			int x = hex % 17;
			hexes[y][x].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
		}
	}

	CombatHex getById(int hex_id) const { return hexes[hex_id / 17][hex_id % 17]; }
	std::vector<int> getHexesInRange(int from, int range) const;

	// checking pathfinding to hexes which arent occupied
	std::vector<int> getReachableHexesFromWalkableHexes(int from, int range, std::vector<int>& hexes, bool can_fly, bool double_wide) const;

	std::vector<int> getWalkableHexesFromList(std::vector<int>& hexes) const; // from hexes in range check which one is not occupied

	std::vector<int> findPath(int from, int to) const;
};