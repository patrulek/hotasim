#pragma once

#include "structures.h"

#include <array>
#include <vector>
#include <unordered_map>

class CombatField;

class CombatPathfinder
{
public:

	int getUnitStartHex(CombatSide _side, int _unit_order, int _units_stacks, bool _double_wide, CombatType _combat_type);

	int distanceBetweenHexes(const int _source_hex, const int _target_hex);
	bool isAdjacentHex(const int _source_hex, const int _target_hex) const;


	std::array<int, 6> getAdjacentHexes(const int _source_hex) const;
	std::array<int, 6> getAdjacentHexesClockwise(const int _source_hex) const;
	std::array<int, 6> getAdjacentHexesCounterClockwise(const int _source_hex) const;

	std::vector<int> getHexesInRange(const int _source_hex, const int _range);
	std::vector<int> getWalkableHexesFromList(std::vector<int>& _hexes, const CombatField& _field); // from hexes in range check which one is not occupied
	// checking pathfinding to hexes which arent occupied
	std::vector<int> getReachableHexesFromWalkableHexes(const int from, const int range, std::vector<int>& hexes, bool can_fly, bool double_wide, const CombatField& _field);
	std::vector<int> findPath(const int _source_hex, const int _target_hex, const CombatField& _field);
private:
	int distanceHash(const int _source_hex, const int _target_hex) const;

	std::unordered_map<int, int> distances;
};

