#include "combat_field.h"

#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <unordered_map>

std::vector<int> CombatField::findPath(int from, int to) const {
	if (!getById(to).isWalkable())
		return std::vector<int>();

	std::vector<int> path{};

	std::vector<int> to_check{from};
	std::vector<int> next_to_check{};
	std::unordered_set<int> checked;
	std::unordered_map<int, int> paths;
	bool found = false;
	paths[from] = -1;

	while (!to_check.empty()) {
		int hex_id = to_check.back();
		to_check.pop_back();
		checked.insert(hex_id);

		if (hex_id == to) {
			found = true;
			break;
		}

		auto adjacent_hexes = getById(hex_id).getAdjacentHexes();
		auto walkable = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		walkable = getWalkableHexesFromList(walkable);

		for (auto hex : walkable) {
			if (paths.find(hex) != std::end(paths)) {
				int distance = hex_id != -1 ? getById(hex_id).distanceToHex(from) : -1;
				int distance2 = paths[hex] != -1 ? getById(paths[hex]).distanceToHex(from) : -1;

				if (distance < distance2)
					paths[hex] = hex_id;
			}
		}

		for (auto check : checked)
			walkable.erase(std::remove(std::begin(walkable), std::end(walkable), check), std::end(walkable));

		checked.insert(std::begin(walkable), std::end(walkable));

		for (auto check : walkable)
			paths[check] = hex_id;

		next_to_check.insert(std::end(next_to_check), std::begin(walkable), std::end(walkable));

		if (to_check.empty()) {
			to_check = next_to_check;
			next_to_check.clear();
		}
	}

	if (found) {
		int start = to;

		path.push_back(start);

		while (start != from) {
			start = paths[start];
			path.push_back(start);
		}

		path.pop_back();
		std::reverse(std::begin(path), std::end(path));
	}

	return path;
}

std::vector<int> CombatField::getReachableHexesFromWalkableHexes(int from, int range, std::vector<int>& hexes, bool can_fly, bool double_wide) const {
	if (can_fly) {
		if (double_wide)
			return std::vector<int>(); // todo
		return hexes; // if can fly and not double_wide, then doesnt need to use pathfinding
	}

	if (double_wide)
		return std::vector<int>(); // todo

	std::vector<int> reachable;

	for (auto hex : hexes) {
		auto path = findPath(from, hex);

		if ( !path.empty() && path.size() <= range)
			reachable.push_back(hex);
	}

	return reachable;
}

std::vector<int> CombatField::getWalkableHexesFromList(std::vector<int>& hexes) const {
	std::vector<int> walkable(hexes);

	for (auto hex : hexes) {
		if ( hex == -1 || !getById(hex).isWalkable())
			walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex), std::end(walkable));
	}

	return walkable;
}

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