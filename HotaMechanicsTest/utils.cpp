#include "utils.h"
#include "../HotaMechanics/combat_field.h"

int getHexId(int row, int col) {
	return row * CombatHex::COLS + col;
}