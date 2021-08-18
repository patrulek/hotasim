#include "combat_pathfinder.h"

#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "combat_field.h"

namespace HotaMechanics {
	using namespace Constants;

	uint64_t CombatPathfinder::cache_access = 0;
	uint64_t CombatPathfinder::cache_misses = 0;

	CombatPathfinder::CombatPathfinder() {
		path.reserve(64);
		reachables.reserve(128);
		initializeAdjacents();
		initializeDistances();
		clearPathCache();
	}

	const uint8_t CombatPathfinder::getUnitStartHex(const CombatSide _side, const int _unit_order, const int _units_stacks,
															  const bool _double_wide, const CombatType _combat_type) const {
		if (_combat_type == CombatType::ENCOUNTER)
			throw std::exception("Not implemented yet");

		// TODO: also formation: loose/tight

		int col = _side == CombatSide::ATTACKER ? 1 + _double_wide : 15 - _double_wide;

		std::vector<int> rows{ 5, -1, -1, -1, -1, -1, -1 };
		if (_units_stacks == 2) { rows[0] = 2; rows[1] = 8; }
		if (_units_stacks == 3) { rows[0] = 2; rows[1] = 5; rows[2] = 8; }
		if (_units_stacks == 4) { throw std::exception("Not implemented yet"); }
		if (_units_stacks == 5) { throw std::exception("Not implemented yet"); }
		if (_units_stacks == 6) { throw std::exception("Not implemented yet"); }
		if (_units_stacks == 7) { rows[0] = 0; rows[1] = 2; rows[2] = 4; rows[3] = 5; rows[4] = 6; rows[5] = 8; rows[6] = 10; }

		int row = rows[_unit_order];
		return row * FIELD_COLS + col;
	}

	const bool CombatPathfinder::areAdjacent(const uint8_t _source_hex, const uint8_t _target_hex) const {
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return false;

		if (_source_hex == _target_hex)
			return false;

		auto& adjacent_hexes = getAdjacentHexes(_source_hex);
		return std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), _target_hex) != std::end(adjacent_hexes);
	}

	void CombatPathfinder::initializeAdjacents() {
		for (uint8_t hex = 0; hex < FIELD_SIZE + 1; ++hex) {
			adjacents[hex] = findAdjacents(hex);
		}
	}

	void CombatPathfinder::initializeDistances() {
		for (uint8_t hex = 0; hex < FIELD_SIZE + 1; ++hex)
			for (uint8_t hex2 = 0; hex2 < FIELD_SIZE + 1; ++hex2) {
				hex_distances[hex][hex2] = distanceBetweenHexes(hex, hex2);
			}
	}

	AdjacentArray CombatPathfinder::findAdjacents(const uint8_t _source_hex) {
		AdjacentArray hexes{ INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID, INVALID_HEX_ID };

		if (_source_hex == INVALID_HEX_ID)
			return hexes;

		bool first_row = _source_hex < FIELD_COLS;
		bool last_row = _source_hex > FIELD_COLS * (FIELD_ROWS - 1) - 1;
		bool first_hex_in_row = _source_hex % FIELD_COLS == 0;
		bool last_hex_in_row = _source_hex % FIELD_COLS == FIELD_COLS - 1;
		bool even_row = (_source_hex / FIELD_COLS % 2) == 1;

		int16_t offset = -1 * even_row;

		bool invalid_left_upper = first_row || (first_hex_in_row && even_row);
		bool invalid_right_upper = first_row || (last_hex_in_row && !even_row);
		bool invalid_left = first_hex_in_row;
		bool invalid_right = last_hex_in_row;
		bool invalid_left_lower = last_row || (first_hex_in_row && even_row);
		bool invalid_right_lower = last_row || (last_hex_in_row && !even_row);

		hexes[0] = (invalid_left_upper * INVALID_HEX_ID) + (!invalid_left_upper * (_source_hex - FIELD_COLS + offset));
		hexes[1] = (invalid_right_upper * INVALID_HEX_ID) + (!invalid_right_upper * (_source_hex - FIELD_COLS + offset + 1));
		hexes[2] = (invalid_left * INVALID_HEX_ID) + (!invalid_left * (_source_hex - 1));
		hexes[3] = (invalid_right * INVALID_HEX_ID) + (!invalid_right * (_source_hex + 1));
		hexes[4] = (invalid_left_lower * INVALID_HEX_ID) + (!invalid_left_lower * (_source_hex + FIELD_COLS + offset));
		hexes[5] = (invalid_right_lower * INVALID_HEX_ID) + (!invalid_right_lower * (_source_hex + FIELD_COLS + offset + 1));

		return hexes;
	}

	const uint8_t CombatPathfinder::getDistanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex) const {
		return hex_distances[_source_hex][_target_hex];
	}

	const AdjacentArray& CombatPathfinder::getAdjacentHexes(const uint8_t _source_hex) const {
		return adjacents[_source_hex];
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesClockwise(const uint8_t _source_hex) const {
		auto& hexes = getAdjacentHexes(_source_hex);
		return AdjacentArray{ hexes[1], hexes[3], hexes[5], hexes[4], hexes[2], hexes[0] };
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesCounterClockwise(const uint8_t _source_hex) const {
		auto& hexes = getAdjacentHexes(_source_hex);
		return AdjacentArray{ hexes[0], hexes[2], hexes[4], hexes[5], hexes[3], hexes[1] };
	}

	std::vector<uint8_t>& CombatPathfinder::getReachableHexesInRange(const uint8_t _source_hex, const uint8_t _range, const CombatField& _field,
																							const bool _can_fly, const bool _double_wide, const bool _ghost_hex) {
		if (_can_fly) {
			if (_double_wide)
				throw std::exception("Not implemented yet");
			throw std::exception("Not implemented yet");
		}

		if (_double_wide)
			throw std::exception("Not implemented yet");

		if (!checked[_source_hex])
			pathMap(_source_hex, _field, _double_wide, _range);

		return reachables;
	}

	const uint8_t CombatPathfinder::distanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex) const {
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return 255;

		if (_target_hex == _source_hex)
			return 0;

		if ((_target_hex % FIELD_COLS == 0) || (_target_hex % FIELD_COLS == FIELD_COLS - 1) 
			||( _source_hex % FIELD_COLS == 0) || (_source_hex % FIELD_COLS == FIELD_COLS - 1))
			return 255;

		if (areAdjacent(_source_hex, _target_hex)) {
			return 1;
		}

		bool same_row = _source_hex / FIELD_COLS == _target_hex / FIELD_COLS;
		if (same_row) {
			return abs(_target_hex - _source_hex);
		}

		bool same_col = _source_hex % FIELD_COLS == _target_hex % FIELD_COLS;
		if (same_col) {
			return abs((_target_hex - _source_hex) / FIELD_COLS);
		}

		int row_dist = abs(_source_hex % FIELD_COLS - _target_hex % FIELD_COLS);
		int col_dist = abs(_source_hex / FIELD_COLS - _target_hex / FIELD_COLS);
		int odd_rows = 0, even_rows = 0;

		for (int y = (_source_hex / FIELD_COLS) + 1; y <= _target_hex / FIELD_COLS; ++y) {
			odd_rows += (y % 2 == 1);
			even_rows += (y % 2 == 0);
		}
		for (int y = (_source_hex / FIELD_COLS) - 1; y >= _target_hex / FIELD_COLS; --y) {
			odd_rows += (y % 2 == 1);
			even_rows += (y % 2 == 0);
		}

		return row_dist + col_dist - (row_dist >= std::max(odd_rows, even_rows) ? std::max(odd_rows, even_rows) : row_dist);
	}

	const uint8_t CombatPathfinder::realDistanceBetweenHexes(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _ghost_hex, const uint8_t _range) {
		if (_target_hex == INVALID_HEX_ID)
			return 255;

		if (_source_hex == _target_hex)
			return 0;

		if (!checked[_target_hex])
			pathMap(_source_hex, _field, false, _range);

		if (_ghost_hex) {
			int dist = 254;
			auto& adjacent = getAdjacentHexes(_target_hex);
			for (uint8_t adj_hex = 0; adj_hex < 6; ++adj_hex)
				if (distances[adjacent[adj_hex]] < dist)
					dist = distances[adjacent[adj_hex]];
			return std::min(dist + 1, 255);
		}

		return distances[_target_hex];
	}

	void CombatPathfinder::clearPathCache() {
		paths.fill(INVALID_HEX_ID);
		distances.fill(255);
		checked.fill(false);
		reachables.clear();
	}

	void CombatPathfinder::storePathCache(const bool _clear) {
		cache_buffer = std::make_tuple(paths, distances, checked, reachables);

		if( _clear)
			clearPathCache();
	}

	void CombatPathfinder::restorePathCache() {
		paths = std::get<0>(cache_buffer);
		distances = std::get<1>(cache_buffer);
		checked = std::get<2>(cache_buffer);
		reachables = std::get<3>(cache_buffer);
	}

	std::vector<uint8_t>& CombatPathfinder::getPathCache(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const uint8_t _range) {
		const bool ghost_target_hex = _field.isHexWalkable(_target_hex, _ghost_hex); 
		
		// if path not found or we need real path, not distance to unit, and target hex is occupied
		if (paths[_target_hex] == INVALID_HEX_ID || !ghost_target_hex)
			return EMPTY_PATH;

		uint8_t start = _target_hex;
		path.clear();
		path.push_back(start);

		while (paths[start] != _source_hex) {
			start = paths[start];
			path.push_back(start);
		}

		if (path.size() > _range)
			return EMPTY_PATH;

		std::reverse(std::begin(path), std::end(path));
		//distance_cache[PathCache(_field, _source_hex, _target_hex, _double_wide)] = (int16_t)path.size();
		return path;
	}

	void CombatPathfinder::pathMap(const uint8_t _source_hex, const CombatField& _field, const bool _double_wide, const uint8_t _range) {
		// just find path to all possible hexes
		uint8_t reachables_cnt = 0;
		FieldArray reachables_;
		uint8_t next_to_check_cnt = 0;
		std::array<uint8_t, 32> next_to_check;
		uint8_t to_check_cnt = 1, dist = 1;
		std::array<uint8_t, 32> to_check{ _source_hex };
		checked[_source_hex] = true;
		const uint8_t range_ = _range + 1;

		while (to_check_cnt > 0 && dist <= range_) {
			uint8_t hex_id = to_check[--to_check_cnt];
			if (dist <= range_) {
				reachables_[reachables_cnt++] = hex_id;
			}

			auto& adjacent_hexes = getAdjacentHexesClockwise(hex_id);
			for (int i = 0; i < 6; ++i) {
				uint8_t adj_hex = adjacent_hexes[i];
				// if target hex is occupied by unit set it also as walkable, but later (after search) check if we need real path to hex or only distance
				const bool is_walkable_hex = _field.getById(adj_hex).isWalkable();
				if (is_walkable_hex && !checked[adj_hex]) {
					next_to_check[next_to_check_cnt++] = adj_hex;
				}
				checked[adj_hex] = true;

				const bool is_closer = is_walkable_hex && (dist < distances[adj_hex]);
				if (is_closer) {
					distances[adj_hex] = dist;
					paths[adj_hex] = hex_id;
				}
			}


			dist += (to_check_cnt == 0);
			for (int hex = 0, end = (to_check_cnt == 0) * next_to_check_cnt; hex < end; ++hex)
				to_check[to_check_cnt++] = next_to_check[--next_to_check_cnt]; // reverse array
		}

		std::move(std::begin(reachables_), std::begin(reachables_) + reachables_cnt, std::back_inserter(reachables));
	}

	std::vector<uint8_t>& CombatPathfinder::findPath(const uint8_t _source_hex, const uint8_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const uint8_t _range) {
		if (_double_wide )
			throw std::exception("Not implemented yet");

		// if already searched for this hex, ant path not found
		if (checked[_target_hex] && paths[_target_hex] == INVALID_HEX_ID)
			return EMPTY_PATH;

		// if one of hexes invalid
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID || _source_hex == _target_hex)
			return EMPTY_PATH;

		// if min distance to hex greater than unit range
		if (_range != 999 && getDistanceBetweenHexes(_source_hex, _target_hex) > _range)
			return EMPTY_PATH;

		// if already found path to that hex, get it from cache
		if (checked[_target_hex]) {
			++CombatPathfinder::cache_access;
			return getPathCache(_source_hex, _target_hex, _field, _double_wide, _ghost_hex, _range);
		}

		++CombatPathfinder::cache_misses;
		pathMap(_source_hex, _field, _double_wide, _range);

		// if found, path will be stored in cache
		return getPathCache(_source_hex, _target_hex, _field, _double_wide, _ghost_hex, _range);
	}
}; // HotaMechanics