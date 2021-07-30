#include "combat_field.h"

#include <unordered_set>
#include <algorithm>
#include <iostream>


std::vector<int> CombatField::getHexesInRange(int from, int range) const {
	std::unordered_set<int> hexes{ from };
	std::vector<int> toCheck{ from };
	std::unordered_set<int> checked{-1};

	while (!toCheck.empty()) {
		int hex_id = toCheck.back();
		checked.insert(hex_id);
		toCheck.pop_back();

		if (hex_id == -1)
			continue;

		if (getById(hex_id).distanceToHex(from) > range)
			continue;

		auto adjacent_hexes = getById(hex_id).getAdjacentHexes();

		hexes.insert(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		toCheck.insert(std::end(toCheck), std::begin(adjacent_hexes), std::end(adjacent_hexes));

		for (auto check : checked)
			toCheck.erase(std::remove(std::begin(toCheck), std::end(toCheck), check), std::end(toCheck));
	}

	hexes.erase(-1);
	std::unordered_set<int> to_remove;
	for (auto hex : hexes)
	{
		//std::cout << "Distance to (" << hex << ") = " << getById(from).distanceToHex(hex) << std::endl;
		if (getById(from).distanceToHex(hex) > range) {
			to_remove.insert(hex);
			//std::cout << "Will remove: " << hex << std::endl;
		}
	}


	for (auto rem : to_remove)
		hexes.erase(rem);

	auto result = std::vector<int>(std::begin(hexes), std::end(hexes));
	std::sort(std::begin(result), std::end(result));
	return result;
}