#pragma once

#include <vector>

struct UnitPermutation {
	int unit_id;
	int unit_order;
	int unit_number;
};

struct ArmyPermutation
{
	std::vector<UnitPermutation> permutations;
};

std::vector<int> getCombatFieldTemplate(const int type = 0);