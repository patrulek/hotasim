#include "utils.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/structures.h"

int getHexId(int row, int col) {
	return row * CombatFieldSize::COLS + col;
}