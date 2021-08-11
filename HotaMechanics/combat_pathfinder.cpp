#include "combat_pathfinder.h"

#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "combat_field.h"

PathCache::PathCache(const HotaMechanics::CombatField& _field, const int16_t _source_hex, const int16_t _target_hex, const bool _double_wide, const bool _ghost_hex)
	: field_hash(_field.getHash()), source_hex(std::min(_source_hex, _target_hex)), target_hex(std::max(_source_hex, _target_hex)), double_wide(_double_wide), ghost_hex(_ghost_hex) {
}

int std::hash<PathCache>::some_val = 0;

namespace HotaMechanics {
	using namespace Constants;

	uint64_t CombatPathfinder::cache_access = 0;
	uint64_t CombatPathfinder::cache_misses = 0;

	CombatPathfinder::CombatPathfinder() {
		path.reserve(64);
		tmp_hexes.resize(128);
		distance_cache.rehash(32768);
		initializeAdjacents();
	}

	const int CombatPathfinder::getUnitStartHex(const CombatSide _side, const int _unit_order, const int _units_stacks, 
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

	const bool CombatPathfinder::areAdjacent(const int16_t _source_hex, const int16_t _target_hex) const {
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return false;

		if (_source_hex == _target_hex)
			return false;

		auto adjacent_hexes = getAdjacentHexes(_source_hex);
		return std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), _target_hex) != std::end(adjacent_hexes);
	}

	void CombatPathfinder::initializeAdjacents() {
		for (int16_t hex = 0; hex < FIELD_SIZE + 1; ++hex)
			adjacents[hex] = findAdjacents(hex);
	}

	AdjacentArray CombatPathfinder::findAdjacents(const int16_t _source_hex) {
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

	const AdjacentArray& CombatPathfinder::getAdjacentHexes(const int16_t _source_hex) const {
		return adjacents[_source_hex];
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesClockwise(const int16_t _source_hex) const {
		auto hexes = getAdjacentHexes(_source_hex);
		return std::array<int16_t, 6>{ hexes[1], hexes[3], hexes[5], hexes[4], hexes[2], hexes[0] };
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesCounterClockwise(const int16_t _source_hex) const {
		auto hexes = getAdjacentHexes(_source_hex);
		return std::array<int16_t, 6>{ hexes[0], hexes[2], hexes[4], hexes[5], hexes[3], hexes[1] };
	}

	const std::vector<int16_t> CombatPathfinder::getReachableAdjacentHexes(const int16_t _adjacent_to, const int16_t _source_hex, const int _range,
																							const CombatField& _field, const bool _can_fly, const bool _double_wide) {
		auto hexes = getAdjacentHexes(_adjacent_to);
		auto hexes_vec = std::vector<int16_t>(std::begin(hexes), std::end(hexes));
		auto walkable_hexes = getWalkableHexesFromList(hexes_vec, _field);
		return getReachableHexesFromWalkableHexes(_source_hex, _range, walkable_hexes, _field, _can_fly, _double_wide);
	}

	const std::vector<int16_t> CombatPathfinder::getHexesInRange(const int16_t _source_hex, const int _range) const {
		if (_source_hex == INVALID_HEX_ID || _range <= 0)
			return EMPTY_VEC;

		if (_range == 1) {
			auto& hexes = getAdjacentHexes(_source_hex);
			return std::vector<int16_t>(std::begin(hexes), std::end(hexes));
		}

		FieldArray to_check; to_check.fill(0);
		int to_check_cnt = 0;
		to_check[to_check_cnt++] = _source_hex;

		FieldArray next_to_check; next_to_check.fill(0);
		int next_to_check_cnt = 0;

		FieldFlagArray checked; checked.fill(0);

		int dist = 0;

		FieldArray hexes;
		int idx = 0;

		while (to_check_cnt > 0 && dist <= _range) {
			const int hex_id = to_check[--to_check_cnt];
			checked[hex_id] = true;
			hexes[idx++] = hex_id;
			
			auto& adjacent_hexes = getAdjacentHexes(hex_id);

			for (auto hex : adjacent_hexes) {
				const bool is_viable_hex = (hex != INVALID_HEX_ID);
				hex = (is_viable_hex * hex) + (!is_viable_hex * INVALID_HEX_ID);

				next_to_check[next_to_check_cnt] = hex;
				next_to_check_cnt += (is_viable_hex * !checked[hex]);
				checked[hex] = true;
			}

			const bool check_next = (to_check_cnt == 0 * next_to_check_cnt > 0);
			to_check_cnt = (!check_next * to_check_cnt) + (check_next * next_to_check_cnt);
			next_to_check_cnt = (!check_next * next_to_check_cnt);
			dist = (!check_next * dist) + (check_next * (dist + 1));

			for (int hex = 0; hex < (check_next * to_check_cnt); ++hex)
				to_check[hex] = (!check_next * to_check[hex]) + (check_next * next_to_check[hex]);
		}

		std::sort(std::begin(hexes), std::begin(hexes) + idx);
		return std::vector<int16_t>(std::begin(hexes), std::begin(hexes) + idx);
	}

	const std::vector<int16_t> CombatPathfinder::getWalkableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field, const bool _ghost_hex) {
		auto hexes = getHexesInRange(_source_hex, _range);
		return getWalkableHexesFromList(hexes, _field, _ghost_hex);
	}

	const std::vector<int16_t> CombatPathfinder::getWalkableHexesFromList(const std::vector<int16_t>& _hexes, const CombatField& _field, const bool _ghost_hex) {
		int16_t idx = 0;
		FieldArray hexes;

		for (auto hex : _hexes) {
			hexes[idx] = hex;
			idx += _field.isHexWalkable(hex, _ghost_hex);
		}
		
		if (idx == 0)
			return EMPTY_VEC;

		return std::vector<int16_t>(std::begin(hexes), std::begin(hexes) + idx);//walkable;
	}

	const std::vector<int16_t> CombatPathfinder::getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																							const bool _can_fly, const bool _double_wide, const bool _ghost_hex) {
		if (_source_hex == INVALID_HEX_ID || _range <= 0)
			return std::vector<int16_t>();
		if (_can_fly || _double_wide)
			throw std::exception("Not implemented yet");

		auto hexes = getWalkableHexesInRange(_source_hex, _range, _field, _ghost_hex);
		return getReachableHexesFromWalkableHexes(_source_hex, _range, hexes, _field, _can_fly, _double_wide, _ghost_hex);
	}


	const std::vector<int16_t> CombatPathfinder::getReachableHexesFromWalkableHexes(const int16_t _source_hex, const int _range,
																										const std::vector<int16_t>& _hexes, const CombatField& _field,
																										const bool _can_fly, const bool _double_wide, const bool _ghost_hex) {
		if (_can_fly) {
			if (_double_wide)
				return std::vector<int16_t>(); // todo
			return _hexes; // if can fly and not double_wide, then doesnt need to use pathfinding
		}

		if (_double_wide)
			return std::vector<int16_t>(); // todo

		std::array<int16_t, 128> reachable;
		int16_t idx = 0;

		for (auto hex : _hexes) {
			auto distance = realDistanceBetweenHexes(_source_hex, hex, _field, _ghost_hex);
			reachable[idx] = hex;
			idx += (distance > 0 && distance <= _range);
		}

		return std::vector<int16_t>(std::begin(reachable), std::begin(reachable) + idx);
	}

	const int16_t CombatPathfinder::distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const {
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return FIELD_SIZE;

		if (_target_hex == _source_hex)
			return 0;

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

	const int16_t CombatPathfinder::realDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _ghost_hex) {
		if (_target_hex == INVALID_HEX_ID)
			return 999;

		if (_source_hex == _target_hex)
			return 0;
		
	/*	if (distance_cache.find(PathCache(_field, _source_hex, _target_hex, false, _ghost_hex)) != std::end(distance_cache)) {

			++CombatPathfinder::cache_access;
			return distance_cache[PathCache(_field, _source_hex, _target_hex, false, _ghost_hex)];
		}*/

	/*	if (distance_cache.load_factor() > 0.98)
			std::cout << "Bucket count: " << distance_cache.bucket_count() << " | Load factor: " << distance_cache.load_factor() << std::endl;*/
		++CombatPathfinder::cache_misses;
		auto dist = (int16_t)findPath(_source_hex, _target_hex, _field, false, _ghost_hex).size();
		return dist;
	}

	const std::vector<int16_t>& CombatPathfinder::findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const int _range) {
		if (_double_wide )
			throw std::exception("Not implemented yet");

		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return EMPTY_PATH;

		if (_range != 999 && distanceBetweenHexes(_source_hex, _target_hex) > _range)
			return EMPTY_PATH;

		if (!_field.isHexWalkable(_target_hex, _ghost_hex) || _source_hex == _target_hex)
			return EMPTY_PATH;

		std::array<int16_t, FIELD_SIZE + 1> to_check; to_check.fill(0);
		int to_check_cnt = 0;
		to_check[to_check_cnt++] = _source_hex;

		std::array<int16_t, FIELD_SIZE + 1> next_to_check; next_to_check.fill(0);
		int next_to_check_cnt = 0;

		std::array<bool, FIELD_SIZE + 1> checked; checked.fill(0);
		std::array<int16_t, FIELD_SIZE + 1> paths; paths.fill(INVALID_HEX_ID);
		std::array<int16_t, FIELD_SIZE + 1> distances; distances.fill(999);

		bool found = false;
		int dist = 1;

		while (to_check_cnt > 0) {
			const int hex_id = to_check[--to_check_cnt];

			if (hex_id == _target_hex) {
				found = true;
				break;
			}

			checked[hex_id] = true;
			auto adjacent_hexes = getAdjacentHexesClockwise(hex_id);

			for (auto hex : adjacent_hexes) {
				const bool is_walkable_hex = _field.isHexWalkable(hex) || (hex == _target_hex && _field.isHexWalkable(hex, _ghost_hex));
				hex = (is_walkable_hex * hex) + (!is_walkable_hex * INVALID_HEX_ID);
				const bool has_path_to_hex = is_walkable_hex && paths[hex] != INVALID_HEX_ID;
				const int distance_to_hex = (has_path_to_hex * distances[hex_id]) + (!has_path_to_hex * 999);
				const bool is_closer = !has_path_to_hex || (dist < distance_to_hex);

				paths[hex] = (hex_id * is_closer) + (paths[hex] * !is_closer);
				distances[hex] = (dist * is_closer) + (distances[hex] * !is_closer);

				next_to_check[next_to_check_cnt] = hex;
				next_to_check_cnt += !checked[hex];
				checked[hex] = true;
			}

			const bool check_next = (to_check_cnt == 0 * next_to_check_cnt > 0);
			to_check_cnt = (!check_next * to_check_cnt) + (check_next * next_to_check_cnt);
			next_to_check_cnt = (!check_next * next_to_check_cnt);
			dist = (!check_next * dist) + (check_next * (dist + 1));

			for (int hex = 0; hex < (check_next * to_check_cnt); ++hex)
				to_check[hex] = (check_next * next_to_check[to_check_cnt - hex - 1]); // reverse array
		}

		if (found) {
			int16_t start = _target_hex;
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

		return EMPTY_PATH;
	}
}; // HotaMechanics