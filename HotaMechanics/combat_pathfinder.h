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

		const uint8_t getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const uint8_t _source_hex, const uint8_t _target_hex) const;
		const Constants::AdjacentArray& getAdjacentHexes(const uint8_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesClockwise(const uint8_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesCounterClockwise(const uint8_t _source_hex) const;

		std::vector<uint8_t>& getReachableHexesInRange(const uint8_t _source_hex, const uint8_t _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const uint8_t getDistanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex) const;
		const uint8_t distanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex) const;
		const uint8_t realDistanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _ghost_hex = false, const uint8_t _range = 254);

		std::vector<uint8_t>& findPath(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const uint8_t _range = 254);
		void pathMap(const uint8_t _source_hex, const CombatField& _field, const bool _double_wide = false, const uint8_t _range = 254);
	private:

		
		void initializeAdjacents();
		void initializeDistances();
		Constants::AdjacentArray findAdjacents(const uint8_t _source_hex);


		std::vector<uint8_t>& getPathCache(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const uint8_t _range = 254);

		std::vector<uint8_t> path;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents;

		using HexDistanceArray = std::array<std::array<uint8_t, Constants::FIELD_SIZE + 1>, Constants::FIELD_SIZE + 1>;
		HexDistanceArray hex_distances;
		// path cache
		Constants::FieldArray paths;
		Constants::FieldArray distances;
		Constants::FieldFlagArray checked;
		std::vector<uint8_t> reachables;
		std::tuple<Constants::FieldArray, Constants::FieldArray, Constants::FieldFlagArray, std::vector<uint8_t>> cache_buffer;
	};
}; // HotaMechanics
