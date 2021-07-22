#pragma once




enum class CombatHexOccupation {
	EMPTY, UNIT, OBSTACLE // etc
};

struct CombatHex {
	unsigned char id;
	CombatHexOccupation occupiedBy;

	CombatHex* getAdjacentHexes() { return nullptr; }
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