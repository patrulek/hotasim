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
	PathCache(const HotaMechanics::CombatField& _field, const int16_t _source_hex, const int16_t _target_hex, const bool _double_wide = false);
	bool operator==(const PathCache& _rhs) const {
		return field_hash == _rhs.field_hash && source_hex == _rhs.source_hex && target_hex == _rhs.target_hex && double_wide == _rhs.double_wide;
	}

	int64_t field_hash{ 0 };
	bool double_wide{ false };
	int16_t source_hex{ HotaMechanics::Constants::INVALID_HEX_ID };
	int16_t target_hex{ HotaMechanics::Constants::INVALID_HEX_ID };
};

namespace std {
	template<>
	struct std::hash<PathCache> {
		std::size_t operator()(const PathCache& _cache) const noexcept {
			std::size_t h1 = std::hash<int16_t>{}(_cache.source_hex);
			h1 ^= (std::hash<int16_t>{}(_cache.target_hex) << (int16_t)_cache.double_wide);
			h1 ^= (_cache.field_hash << (int16_t)!_cache.double_wide);
			return h1;
		}
	};
}; // std

namespace HotaMechanics {

	class CombatPathfinder
	{


	public:
		CombatPathfinder();


		const int getUnitStartHex(const Constants::CombatSide _side, const int _unit_order, const int _units_stacks, 
										  const bool _double_wide, const Constants::CombatType _combat_type) const;

		const bool areAdjacent(const int16_t _source_hex, const int16_t _target_hex) const;
		const Constants::AdjacentArray& getAdjacentHexes(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesClockwise(const int16_t _source_hex) const;
		const Constants::AdjacentArray getAdjacentHexesCounterClockwise(const int16_t _source_hex) const;

		const std::vector<int16_t> getReachableAdjacentHexes(const int16_t _adjacent_to, const int16_t _source_hex, const int _range,
																		const CombatField& _field, const bool _can_fly, const bool _double_wide);

		const std::vector<int16_t> getHexesInRange(const int16_t _source_hex, const int _range) const;
		// from hexes in range check which one are not occupied
		const std::vector<int16_t> getWalkableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field, const bool _ghost_hex = false);
		// checking pathfinding to hexes which arent occupied
		const std::vector<int16_t> getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																		const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		const int16_t distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const;
		const int16_t realDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _ghost_hex = false);
		const std::vector<int16_t>& findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide = false, const bool _ghost_hex = false, const int _range = 999);
	private:
		const std::vector<int16_t> getWalkableHexesFromList(const std::vector<int16_t>& _hexes, const CombatField& _field, const bool _ghost_hex = false);
		const std::vector<int16_t> getReachableHexesFromWalkableHexes(const int16_t _source_hex, const int _range, const std::vector<int16_t>& _hexes,
																					 const CombatField& _field, const bool _can_fly, const bool _double_wide, const bool _ghost_hex = false);

		void initializeAdjacents();
		Constants::AdjacentArray findAdjacents(const int16_t _source_hex);

		std::vector<int16_t> path;
		std::vector<int16_t> tmp_hexes;
		std::unordered_map<PathCache, int16_t> distance_cache;
		std::array<Constants::AdjacentArray, Constants::FIELD_SIZE + 1> adjacents;
		static int cache_access;
	};
}; // HotaMechanics
