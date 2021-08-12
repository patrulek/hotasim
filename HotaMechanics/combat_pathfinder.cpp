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
		tmp_hexes.resize(128);
		range_cache.rehash(4096);
		initializeAdjacents();
		initializePathMoves();
		initializeDistances();
		clearPathCache();
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

	void CombatPathfinder::initializePathMoves() {
		for( int16_t hex = 0; hex < FIELD_SIZE + 1; ++hex)
			for (int16_t hex2 = 0; hex2 < FIELD_SIZE + 1; ++hex2) {
				if (hex == hex2)
					continue;
				path_moves[hex][hex2] = nextPathMove(hex, hex2);
			}
	}

	void CombatPathfinder::initializeDistances() {
		for (int16_t hex = 0; hex < FIELD_SIZE + 1; ++hex)
			for (int16_t hex2 = 0; hex2 < FIELD_SIZE + 1; ++hex2) {
				hex_distances[hex][hex2] = distanceBetweenHexes(hex, hex2);
			}
	}

	const int8_t CombatPathfinder::getNextPathMove(const int16_t _source_hex, const int16_t _target_hex) const {
		return path_moves[_source_hex][_target_hex];
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

	const int16_t CombatPathfinder::getDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const {
		return hex_distances[_source_hex][_target_hex];
	}

	const AdjacentArray& CombatPathfinder::getAdjacentHexes(const int16_t _source_hex) const {
		return adjacents[_source_hex];
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesClockwise(const int16_t _source_hex) const {
		auto& hexes = getAdjacentHexes(_source_hex);
		return std::array<int16_t, 6>{ hexes[1], hexes[3], hexes[5], hexes[4], hexes[2], hexes[0] };
	}

	const AdjacentArray CombatPathfinder::getAdjacentHexesCounterClockwise(const int16_t _source_hex) const {
		auto& hexes = getAdjacentHexes(_source_hex);
		return std::array<int16_t, 6>{ hexes[0], hexes[2], hexes[4], hexes[5], hexes[3], hexes[1] };
	}

	std::vector<int16_t> CombatPathfinder::getHexesInRange(const int16_t _source_hex, const int16_t _range) {
		if (_source_hex == INVALID_HEX_ID || _range <= 0)
			return EMPTY_VEC;

		if (_range == 1) {
			auto& hexes = getAdjacentHexes(_source_hex);
			return std::vector<int16_t>(std::begin(hexes), std::end(hexes));
		}

		if (range_cache.find(_source_hex | (_range << 8)) != std::end(range_cache))
			return range_cache[_source_hex | (_range << 8)];

		FieldArray to_check; to_check.fill(0);
		int to_check_cnt = 0;
		to_check[to_check_cnt++] = _source_hex;

		FieldArray next_to_check; next_to_check.fill(0);
		int next_to_check_cnt = 0;

		FieldFlagArray checked; checked.fill(0);

		int dist = 0;

		FieldArray hexes; hexes.fill(0);

		while (to_check_cnt > 0 && dist <= _range) {
			const int hex_id = to_check[--to_check_cnt];
			checked[hex_id] = true;
			hexes[hexes[FIELD_SIZE]++] = hex_id;
			
			auto& adjacent_hexes = getAdjacentHexes(hex_id);

			for (auto hex : adjacent_hexes) {
				const bool is_viable_hex = (hex != INVALID_HEX_ID);
				next_to_check[next_to_check_cnt] = hex;
				next_to_check_cnt += (is_viable_hex * !checked[hex]);
				checked[hex] = true;
			}

			dist += (to_check_cnt == 0);

			for (int hex = 0, end = (to_check_cnt == 0) * next_to_check_cnt; hex < end; ++hex)
				to_check[to_check_cnt++] = next_to_check[--next_to_check_cnt];
		}

		std::sort(std::begin(hexes), std::begin(hexes) + hexes[FIELD_SIZE]);
		auto result = std::vector<int16_t>(std::begin(hexes), std::begin(hexes) + hexes[FIELD_SIZE]);
		range_cache[_source_hex | (_range << 8)] = result;
		return result;
	}

	std::vector<int16_t> CombatPathfinder::getWalkableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field, const bool _ghost_hex) {
		return getWalkableHexesFromList(getHexesInRange(_source_hex, _range), _field, _ghost_hex);
	}

	std::vector<int16_t> CombatPathfinder::getWalkableHexesFromList(std::vector<int16_t>&& _hexes, const CombatField& _field, const bool _ghost_hex) {
		int16_t idx = 0;

		for (auto hex : _hexes) {
			_hexes[idx] = hex;
			idx += _field.isHexWalkable(hex, _ghost_hex);
		}
		
		if (idx == 0)
			return EMPTY_VEC;

		_hexes.resize(idx);
		return _hexes;
	}

	std::vector<int16_t> CombatPathfinder::getReachableHexesInRange(const int16_t _source_hex, const int _range, const CombatField& _field,
																							const bool _can_fly, const bool _double_wide, const bool _ghost_hex) {
		if (_source_hex == INVALID_HEX_ID || _range <= 0)
			return std::vector<int16_t>();
		if (_can_fly || _double_wide)
			throw std::exception("Not implemented yet");

		return getReachableHexesFromWalkableHexes(_source_hex, _range, getHexesInRange(_source_hex, _range), _field, _can_fly, _double_wide, _ghost_hex);
	}


	std::vector<int16_t> CombatPathfinder::getReachableHexesFromWalkableHexes(const int16_t _source_hex, const int _range,
																										std::vector<int16_t>&& _hexes, const CombatField& _field,
																										const bool _can_fly, const bool _double_wide, const bool _ghost_hex) {
		if (_can_fly) {
			if (_double_wide)
				return std::vector<int16_t>(); // todo
			return _hexes; // if can fly and not double_wide, then doesnt need to use pathfinding
		}

		if (_double_wide)
			return std::vector<int16_t>(); // todo

		int16_t idx = 0;

		for (auto hex : _hexes) {
			auto distance = realDistanceBetweenHexes(_source_hex, hex, _field, _ghost_hex, _range);
			_hexes[idx] = hex;// | (distance << 8);
			idx += (distance > 0 && distance <= _range);
		}

		_hexes.resize(idx);
		return _hexes;
	}

	const int16_t CombatPathfinder::distanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex) const {
		if (_source_hex == INVALID_HEX_ID || _target_hex == INVALID_HEX_ID)
			return 999;

		if (_target_hex == _source_hex)
			return 0;

		if ((_target_hex % FIELD_COLS == 0) || (_target_hex % FIELD_COLS == FIELD_COLS - 1) 
			||( _source_hex % FIELD_COLS == 0) || (_source_hex % FIELD_COLS == FIELD_COLS - 1))
			return 999;

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

	const int16_t CombatPathfinder::realDistanceBetweenHexes(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _ghost_hex, const int16_t _range) {
		if (_target_hex == INVALID_HEX_ID)
			return 999;

		if (_source_hex == _target_hex)
			return 0;

		if (!checked[_target_hex])
			pathMap(_source_hex, _field, false, _range);
		
		if (distances[_target_hex] != 999)
			return distances[_target_hex];

		if (_ghost_hex) {
			int dist = 999;
			auto& adjacent = getAdjacentHexes(_target_hex);
			for (auto adj_hex : adjacent)
				if (distances[adj_hex] < dist)
					dist = distances[adj_hex];
			return std::min(dist + 1, 999);
		}

		return 999;
	}

	const int16_t CombatPathfinder::nextPathMove(const int16_t _source_hex, const int16_t _target_hex) {
		int x_diff = (_target_hex % FIELD_COLS) - (_source_hex % FIELD_COLS);
		int y_diff = (_target_hex / FIELD_COLS) - (_source_hex / FIELD_COLS);

		bool even_row = (_source_hex / FIELD_COLS % 2) == 1;

		bool upper_row = y_diff < 0; // we go up
		bool lower_row = y_diff > 0; // we go down
		bool same_row = y_diff == 0;
		bool further_col = x_diff > 0; // we go right
		bool closer_col = x_diff < 0; // we go left
		bool same_col = x_diff == 0;

		if (same_row && further_col) return 3;
		if (same_row && closer_col) return 2;
		if (upper_row && further_col) return 1;
		if (upper_row && same_col) return even_row;
		if (upper_row && closer_col && abs(x_diff) > abs(std::ceil(y_diff / 2.0))) return 2;
		if (lower_row && further_col) return 5 - 2 * (abs(x_diff) > abs(std::ceil(y_diff / 2.0)));
		if (lower_row && same_col) return 4 + ((even_row && abs(y_diff) % 2 == 1) || (!even_row && abs(y_diff) % 2 == 0));
		
		return 4 + (abs(x_diff) < abs(std::ceil(y_diff / 2.0)));
	}

	std::vector<int16_t>& CombatPathfinder::findSimplePath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const int _range) {
		int16_t cur_hex = _source_hex;
		int dist = 0;
		std::array<int16_t, 32> p; p.fill(INVALID_HEX_ID);
		bool found = true;
		bool ghost_target_hex = _field.isHexWalkable(_target_hex, true);

		while (found) {
			int next_move = getNextPathMove(cur_hex, _target_hex);
			auto& adjacent = getAdjacentHexes(cur_hex);

			bool is_hex_walkable = _field.isHexWalkable(adjacent[next_move]) || (adjacent[next_move] == _target_hex && ghost_target_hex);
			p[dist++] = adjacent[next_move];
			cur_hex = adjacent[next_move];
			found = !((cur_hex == _target_hex && ghost_target_hex) || !is_hex_walkable);
		}

		if (cur_hex == _target_hex && _field.isHexWalkable(_target_hex, _ghost_hex)) {
			path.clear();

			for (int i = 0; i < dist; ++i)
				path.push_back(p[i]);

			return path;
		}

		return EMPTY_PATH;
	}

	void CombatPathfinder::clearPathCache() {
		paths.fill(INVALID_HEX_ID);
		distances.fill(999);
		checked.fill(false);
	}

	void CombatPathfinder::storePathCache() {
		cache_buffer = std::make_tuple(paths, distances, checked);
		clearPathCache();
	}

	void CombatPathfinder::restorePathCache() {
		paths = std::get<0>(cache_buffer);
		distances = std::get<1>(cache_buffer);
		checked = std::get<2>(cache_buffer);
	}

	std::vector<int16_t>& CombatPathfinder::getPathCache(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const int _range) {
		bool ghost_target_hex = _field.isHexWalkable(_target_hex, _ghost_hex); 
		
		// if path not found or we need real path, not distance to unit, and target hex is occupied
		if (paths[_target_hex] == INVALID_HEX_ID || !ghost_target_hex)
			return EMPTY_PATH;

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

	void CombatPathfinder::pathMap(const int16_t _source_hex, const CombatField& _field, const bool _double_wide, const int16_t _range) {
		// just find path to all possible hexes
		std::array<int16_t, FIELD_SIZE + 1> to_check; to_check.fill(0);
		std::array<int16_t, FIELD_SIZE + 1> next_to_check; next_to_check.fill(0);
		int to_check_cnt = 1, next_to_check_cnt = 0;
		to_check[0] = _source_hex;

		int dist = 1;

		while (to_check_cnt > 0 && dist <= _range) {
			const int hex_id = to_check[--to_check_cnt];
			checked[hex_id] = true;

			auto& adjacent_hexes = getAdjacentHexesClockwise(hex_id);

			for (auto adj_hex : adjacent_hexes) {
				// if target hex is occupied by unit set it also as walkable, but later (after search) check if we need real path to hex or only distance
				const bool is_walkable_hex = _field.isHexWalkable(adj_hex, false);
				adj_hex = (is_walkable_hex * adj_hex) + (!is_walkable_hex * INVALID_HEX_ID);
				const bool has_path_to_hex = paths[adj_hex] != INVALID_HEX_ID;
				const bool is_closer = !has_path_to_hex || (dist < distances[hex_id]);

				paths[adj_hex] = (hex_id * is_closer) + (paths[adj_hex] * !is_closer);
				distances[adj_hex] = (dist * is_closer) + (distances[adj_hex] * !is_closer);

				next_to_check[next_to_check_cnt] = adj_hex;
				next_to_check_cnt += !checked[adj_hex];
				checked[adj_hex] = true;
			}

			dist += (to_check_cnt == 0);
			for (int hex = 0, end = (to_check_cnt == 0) * next_to_check_cnt; hex < end; ++hex)
				to_check[to_check_cnt++] = next_to_check[--next_to_check_cnt]; // reverse array
		}
	}

	std::vector<int16_t>& CombatPathfinder::findPath(const int16_t _source_hex, const int16_t _target_hex, const CombatField& _field, const bool _double_wide, const bool _ghost_hex, const int _range) {
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


		// check if there is simple path, if so, return it
		//if (!findSimplePath(_source_hex, _target_hex, _field, _double_wide, _ghost_hex, _range).empty())
		//	return path;

		// just find path

		//std::array<int16_t, FIELD_SIZE + 1> to_check; to_check.fill(0);
		//std::array<int16_t, FIELD_SIZE + 1> next_to_check; next_to_check.fill(0);
		//int to_check_cnt = 0, next_to_check_cnt = 0;
		//to_check[to_check_cnt++] = _source_hex;

		//int dist = 1;
		//bool ghost_target_hex = _field.isHexWalkable(_target_hex, true);

		//FieldArray paths_; paths_.fill(INVALID_HEX_ID);
		//FieldArray distances_; distances_.fill(999);
		//FieldFlagArray checked_; checked_.fill(false);

		//while (to_check_cnt > 0) {
		//	const int hex_id = to_check[--to_check_cnt];
		//	checked[hex_id] = true;

		//	auto& adjacent_hexes = getAdjacentHexesClockwise(hex_id);

		//	for (auto adj_hex : adjacent_hexes) {
		//		// if target hex is occupied by unit set it also as walkable, but later (after search) check if we need real path to hex or only distance
		//		const bool is_walkable_hex = _field.isHexWalkable(adj_hex) || (adj_hex == _target_hex && ghost_target_hex);
		//		adj_hex = (is_walkable_hex * adj_hex) + (!is_walkable_hex * INVALID_HEX_ID);
		//		const bool has_path_to_hex = paths_[adj_hex] != INVALID_HEX_ID;
		//		const bool is_closer = !has_path_to_hex || (dist < distances_[hex_id]);

		//		paths_[adj_hex] = (hex_id * is_closer) + (paths_[adj_hex] * !is_closer);
		//		distances_[adj_hex] = (dist * is_closer) + (distances_[adj_hex] * !is_closer);

		//		next_to_check[next_to_check_cnt] = adj_hex;
		//		next_to_check_cnt += !checked_[adj_hex];
		//		checked_[adj_hex] = true;

		//		bool found = adj_hex == _target_hex;
		//		to_check_cnt = -found + (!found * to_check_cnt);

		//		checked[adj_hex] = checked_[adj_hex];
		//		paths[adj_hex] = paths_[adj_hex];
		//		distances[adj_hex] = distances_[adj_hex];
		//	}

		//	dist += (to_check_cnt == 0);
		//	for (int hex = 0, end = (to_check_cnt == 0) * next_to_check_cnt; hex < end; ++hex)
		//		to_check[to_check_cnt++] = next_to_check[--next_to_check_cnt]; // reverse array
		//}

		// if found, path will be stored in cache
		return getPathCache(_source_hex, _target_hex, _field, _double_wide, _ghost_hex, _range);
	}
}; // HotaMechanics