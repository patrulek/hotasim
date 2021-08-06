#pragma once

#include "structures.h"

#include <array>
#include <vector>

namespace HotaMechanics {
	class CombatField;

	class CombatPathfinder
	{
	public:

		const int getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const int _source_hex, const int _target_hex) const;
		const std::array<int, 6> getAdjacentHexes(const int _source_hex) const;
		const std::array<int, 6> getAdjacentHexesClockwise(const int _source_hex) const;
		const std::array<int, 6> getAdjacentHexesCounterClockwise(const int _source_hex) const;

		const std::vector<int> getReachableAdjacentHexes(const int _adjacent_to, const int _source_hex, const int _range,
																		const CombatField& _field, const bool _can_fly, const bool _double_wide) const;

		const std::vector<int> getHexesInRange(const int _source_hex, const int _range) const;
		// from hexes in range check which one are not occupied
		const std::vector<int> getWalkableHexesInRange(const int _source_hex, const int _range, const CombatField& _field) const;
		// checking pathfinding to hexes which arent occupied
		const std::vector<int> getReachableHexesInRange(const int _source_hex, const int _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide) const;

		const int distanceBetweenHexes(const int _source_hex, const int _target_hex) const;
		const std::vector<int> findPath(const int _source_hex, const int _target_hex, const CombatField& _field, const bool _double_wide = false) const;
	private:
		const std::vector<int> getWalkableHexesFromList(const std::vector<int>& _hexes, const CombatField& _field) const;
		const std::vector<int> getReachableHexesFromWalkableHexes(const int from, const int range, const std::vector<int>& hexes,
																					 const CombatField& _field, const bool can_fly, const bool double_wide) const;
	};
}; // HotaMechanics
