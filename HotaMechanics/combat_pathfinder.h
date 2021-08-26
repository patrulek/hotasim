#pragma once

#include "structures.h"
#include "utils.h"

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

		const HexId getUnitStartHex(const Constants::CombatSide _side, const uint8_t _unit_order, const int16_t _units_stacks,
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const HexId _source_hex, const HexId _target_hex) const;
		const Constants::AdjacentArray& getAdjacentHexes(const HexId _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesClockwise(const HexId _source_hex) const;

		std::vector<HexId>& getReachableHexesInRange(const HexId _source_hex, const uint8_t _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const uint8_t getDistanceBetweenHexes(const HexId _source_hex, const HexId _target_hex) const;
		const uint8_t distanceBetweenHexes(const HexId _source_hex, const HexId _target_hex) const;
		const uint8_t realDistanceBetweenHexes(const HexId _source_hex, const HexId _target_hex, const CombatField& _field, const bool _ghost_hex = false, const uint8_t _range = Constants::MAX_FIELD_RANGE);

		std::vector<HexId>& findPath(const HexId _source_hex, const HexId _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const uint8_t _range = Constants::MAX_FIELD_RANGE);
		void pathMap(const HexId _source_hex, const CombatField& _field, const bool _double_wide = false, const uint8_t _range = Constants::MAX_FIELD_RANGE);
	private:
		void initializeAdjacents();
		void initializeDistances();
		Constants::AdjacentArray findAdjacents(const HexId _source_hex);

		std::vector<HexId>& getPathCache(const HexId _source_hex, const HexId _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const uint8_t _range = Constants::MAX_FIELD_RANGE);

		std::vector<HexId> path;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents_clockwise;

		std::unordered_map<Hash, std::tuple<Constants::FieldArray, Constants::FieldArray, Constants::FieldFlagArray, std::vector<HexId>>> pathmap_cache;
		std::unordered_map<Hash, std::pair<HexId, Utils::HexSet>> pathmap_temp_cache;

		using HexDistanceArray = std::array<std::array<uint8_t, Constants::FIELD_SIZE + 1>, Constants::FIELD_SIZE + 1>;
		HexDistanceArray hex_distances;
		// path cache
		Constants::FieldArray paths;
		Constants::FieldArray distances;
		Constants::FieldFlagArray checked;
		std::vector<HexId> reachables;
		std::tuple<Constants::FieldArray, Constants::FieldArray, Constants::FieldFlagArray, std::vector<HexId>> cache_buffer;
	};
}; // HotaMechanics
