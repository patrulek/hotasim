#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <iostream>

#include "structures.h"

enum class CombatHexOccupation {
	EMPTY, UNIT, SOFT_OBSTACLE, SOLID_OBSTACLE // etc
};

class CombatHex {

	int id;
	CombatHexOccupation occupied_by;

public:

	int getId() const { return id; }
	void setId(const int _id) { id = _id; }
	CombatHexOccupation getOccupation() const { return occupied_by; }

	void occupyHex(CombatHexOccupation type) {
		occupied_by = type;
	}

	bool isWalkable() const {
		bool first_col = id % CombatFieldSize::COLS == 0;
		bool last_col = id % CombatFieldSize::COLS == CombatFieldSize::COLS - 1;

		return !(occupied_by == CombatHexOccupation::SOLID_OBSTACLE || occupied_by == CombatHexOccupation::UNIT)
			&& !(first_col || last_col);
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

	explicit CombatField(const CombatFieldType _field_type)
			: combatFieldId(_field_type) {
		
		initializeCombatHexes();
		//if (combatFieldTemplate == CombatFieldTemplate::TMP1)
		//	hexes[0][5].occupiedBy = CombatHexOccupation::SOLID_OBSTACLE;
	}

	CombatField() = delete;

	void initializeCombatHexes() {
		for (int y = 0; y < CombatFieldSize::ROWS; ++y) {
			for (int x = 0; x < CombatFieldSize::COLS; ++x) {
				hexes[y][x].setId(y * CombatFieldSize::COLS + x);
				hexes[y][x].occupyHex(CombatHexOccupation::EMPTY);
			}
		}
	}

	void fillHex(int _target_hex, CombatHexOccupation _occupied_by) {
		hexes[_target_hex / CombatFieldSize::COLS][_target_hex % CombatFieldSize::COLS].occupyHex(_occupied_by);
	}

	void clearHex(int _target_hex) {
		fillHex(_target_hex, CombatHexOccupation::EMPTY);
	}
		
	void setTemplate(std::vector<int>& _template) {
		if (_template.size() != CombatFieldSize::ROWS * CombatFieldSize::COLS)
			throw std::exception("Wrong template size");

		int id = 0;

		for (auto hex : _template) {
			int y = id / CombatFieldSize::COLS;
			int x = (id++) % CombatFieldSize::COLS;

			if (hex)
				hexes[y][x].occupyHex(CombatHexOccupation::SOLID_OBSTACLE);
			else
				hexes[y][x].occupyHex(CombatHexOccupation::EMPTY);
		}
	}

	CombatHex getById(int hex_id) const { return hexes[hex_id / CombatFieldSize::COLS][hex_id % CombatFieldSize::COLS]; }


};