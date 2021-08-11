#pragma once

#include "structures.h"

#include <array>
#include <vector>
#include <unordered_map>
#include <functional>

namespace HotaMechanics {
	class CombatField;
}

struct PathCache {
	PathCache(const HotaMechanics::CombatField& _field, const int16_t _source_hex, const int16_t _target_hex, const bool _double_wide = false, const bool _ghost_hex = false);
	bool operator==(const PathCache& _rhs) const {
		return field_hash == _rhs.field_hash && source_hex == _rhs.source_hex && target_hex == _rhs.target_hex && double_wide == _rhs.double_wide && ghost_hex == _rhs.ghost_hex;
	}

	int64_t field_hash{ 0 };
	bool double_wide{ false };
	bool ghost_hex{ false };
	int16_t source_hex{ HotaMechanics::Constants::INVALID_HEX_ID };
	int16_t target_hex{ HotaMechanics::Constants::INVALID_HEX_ID };
};

namespace std {
	template<>
	struct std::hash<PathCache> {
		static int some_val;

		std::size_t operator()(const PathCache& _cache) const noexcept {
			//int64_t to_hash = _cache.field_hash | ((int64_t)_cache.source_hex << 44) | ((int64_t)_cache.target_hex << 48) | ((int64_t)_cache.double_wide << 63);
			//return std::hash<int64_t>{}(to_hash);

			std::size_t h1 = std::hash<int16_t>{}(_cache.source_hex);
			h1 ^= std::hash<int16_t>{}(_cache.target_hex);
			h1 ^= std::hash<int64_t>{}(_cache.field_hash);
			h1 ^= std::hash<bool>{}(_cache.ghost_hex);
			return h1;
		}
	};
}; // std

namespace HotaMechanics {

	class CombatPathfinder
	{


	public:
		CombatPathfinder();
		void clearCache() { distance_cache.clear(); }
		static uint64_t cache_access;
		static uint64_t cache_misses;

		const int getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const int16_t _source_hex, const int16_t _target_hex) const;
		const Constants::AdjacentArray& getAdjacentHexes(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesClockwise(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesCounterClockwise(const int16_t _source_hex) const;

		std::vector<int16_t> getReachableAdjacentHexes(const int16_t _adjacent_to, const int16_t _source_hex, const int _range,
																		const CombatField& _field, const bool _can_fly, const bool _double_wide);

		std::vector<int16_t> getHexesInRange(const int16_t _source_hex, const int16_t _range);
		// from hexes in range check which one are not occupied
		std::vector<int16_t> getWalkableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field, const bool _ghost_hex = false);
		// checking pathfinding to hexes which arent occupied
		std::vector<int16_t> getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const int16_t distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const;
		const int16_t realDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _ghost_hex = false);

		std::vector<int16_t>& findSimplePath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 999);
		std::vector<int16_t>& findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 999);
	private:
		const int16_t nextPathMove(const int16_t _source_hex, const int16_t _target_hex);

		std::vector<int16_t> getWalkableHexesFromList(const std::vector<int16_t>& _hexes, const CombatField& _field, const bool _ghost_hex = false);
		std::vector<int16_t> getReachableHexesFromWalkableHexes(const int16_t _source_hex, const int _range, std::vector<int16_t>& _hexes,
																					 const CombatField& _field, const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		void initializeAdjacents();
		Constants::AdjacentArray findAdjacents(const int16_t _source_hex);

		std::vector<int16_t> path;
		std::vector<int16_t> tmp_hexes;
		std::unordered_map<PathCache, int16_t> distance_cache;
		std::unordered_map<int16_t, std::vector<int16_t>> range_cache;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents;
	};
}; // HotaMechanics
