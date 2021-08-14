#pragma once

#include "structures.h"

#include <array>
#include <vector>
#include <unordered_map>
#include <functional>

namespace HotaMechanics {
	class CombatField;
}

namespace HotaMechanics {

	class CombatPathfinder
	{


	public:
		CombatPathfinder();
		void clearPathCache();
		void storePathCache(const bool _clear = true);
		void restorePathCache();
		static uint64_t cache_access;
		static uint64_t cache_misses;

		const int getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const int16_t _source_hex, const int16_t _target_hex) const;
		const Constants::AdjacentArray& getAdjacentHexes(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesClockwise(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesCounterClockwise(const int16_t _source_hex) const;

		std::vector<int16_t> getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const int16_t getDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const;
		const int16_t distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const;
		const int16_t realDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _ghost_hex = false, const int16_t _range = 999);

		std::vector<int16_t>& findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 998);
		void pathMap(const int16_t _source_hex, const CombatField& _field, const bool _double_wide = false, const int16_t _range = 999);
	private:

		
		void initializeAdjacents();
		void initializeDistances();
		Constants::AdjacentArray findAdjacents(const int16_t _source_hex);

		std::vector<int16_t>& getPathCache(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 998);

		std::vector<int16_t> path;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents;

		using HexDistanceArray = std::array<std::array<int16_t, Constants::FIELD_SIZE + 1>, Constants::FIELD_SIZE + 1>;
		HexDistanceArray hex_distances;
		// path cache
		Constants::FieldArray paths;
		Constants::FieldArray distances;
		Constants::FieldFlagArray checked;
		std::vector<int16_t> reachables;
		std::tuple<Constants::FieldArray, Constants::FieldArray, Constants::FieldFlagArray, std::vector<int16_t>> cache_buffer;
	};
}; // HotaMechanics
