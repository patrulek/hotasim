#pragma once

#include "structures.h"

#include <array>
#include <vector>
#include <unordered_map>

namespace HotaMechanics {
	class CombatField;

	class CombatPathfinder
	{

	public:
		CombatPathfinder();


		const int getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const int16_t _source_hex, const int16_t _target_hex) const;
		const std::array<int16_t, 6> getAdjacentHexes(const int16_t _source_hex) const;
		const std::array<int16_t, 6> getAdjacentHexesClockwise(const int16_t _source_hex) const;
		const std::array<int16_t, 6> getAdjacentHexesCounterClockwise(const int16_t _source_hex) const;

		const std::vector<int16_t> getReachableAdjacentHexes(const int16_t _adjacent_to, const int16_t _source_hex, const int _range,
																		const CombatField& _field, const bool _can_fly, const bool _double_wide);

		const std::vector<int16_t> getHexesInRange(const int16_t _source_hex, const int _range) const;
		// from hexes in range check which one are not occupied
		const std::vector<int16_t> getWalkableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field, const bool _ghost_hex = false) const;
		// checking pathfinding to hexes which arent occupied
		const std::vector<int16_t> getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const int16_t distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const;
		const std::vector<int16_t>& findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 999);
	private:
		const std::vector<int16_t> getWalkableHexesFromList(const std::vector<int16_t>& _hexes, const CombatField& _field, const bool _ghost_hex = false) const;
		const std::vector<int16_t> getReachableHexesFromWalkableHexes(const int16_t _source_hex, const int _range, const std::vector<int16_t>& _hexes,
																					 const CombatField& _field, const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);


		std::vector<int16_t> path;
	};
}; // HotaMechanics
