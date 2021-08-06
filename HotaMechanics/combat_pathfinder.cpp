#include "combat_pathfinder.h"

#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "combat_field.h"

namespace HotaMechanics {
	using namespace Constants;

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

	const bool CombatPathfinder::areAdjacent(const int _source_hex, const int _target_hex) const {
		if (_source_hex == -1 || _target_hex == -1)
			throw std::exception("Invalid hex id");

		if (_source_hex == _target_hex)
			return false;

		auto adjacent_hexes = getAdjacentHexes(_source_hex);
		return std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), _target_hex) != std::end(adjacent_hexes);
	}

	const std::array<int, 6> CombatPathfinder::getAdjacentHexes(const int _source_hex) const {
		if (_source_hex == -1)
			throw std::exception("Invalid hex id");

		std::array<int, 6> hexes{ -1, -1, -1, -1, -1, -1 };
		int offset = -1 * (_source_hex / FIELD_COLS % 2 != 0);

		bool first_row = _source_hex < FIELD_COLS;
		bool last_row = _source_hex > FIELD_COLS * (FIELD_ROWS - 1) - 1;
		bool first_hex_in_row = _source_hex % FIELD_COLS == 0;
		bool last_hex_in_row = _source_hex % FIELD_COLS == FIELD_COLS - 1;
		bool even_row = _source_hex / FIELD_COLS % 2 == 1;

		hexes[0] = first_row || (first_hex_in_row && even_row) ? -1 : _source_hex - FIELD_COLS + offset;
		hexes[1] = first_row || (last_hex_in_row && !even_row) ? -1 : _source_hex - FIELD_COLS + offset + 1;
		hexes[2] = first_hex_in_row ? -1 : _source_hex - 1;
		hexes[3] = last_hex_in_row ? -1 : _source_hex + 1;
		hexes[4] = last_row || (first_hex_in_row && even_row) ? -1 : _source_hex + FIELD_COLS + offset;
		hexes[5] = last_row || (last_hex_in_row && !even_row) ? -1 : _source_hex + FIELD_COLS + offset + 1;

		return hexes;
	}

	const std::array<int, 6> CombatPathfinder::getAdjacentHexesClockwise(const int _source_hex) const {
		auto hexes = getAdjacentHexes(_source_hex);
		return std::array<int, 6>{ hexes[1], hexes[3], hexes[5], hexes[4], hexes[2], hexes[0] };
	}

	const std::array<int, 6> CombatPathfinder::getAdjacentHexesCounterClockwise(const int _source_hex) const {
		auto hexes = getAdjacentHexes(_source_hex);
		return std::array<int, 6>{ hexes[0], hexes[2], hexes[4], hexes[5], hexes[3], hexes[1] };
	}

	const std::vector<int> CombatPathfinder::getReachableAdjacentHexes(const int _adjacent_to, const int _source_hex, const int _range,
																							const CombatField& _field, const bool _can_fly, const bool _double_wide) const {
		auto hexes = getAdjacentHexes(_adjacent_to);
		auto hexes_vec = std::vector<int>(std::begin(hexes), std::end(hexes));
		auto walkable_hexes = getWalkableHexesFromList(hexes_vec, _field);
		return getReachableHexesFromWalkableHexes(_source_hex, _range, walkable_hexes, _field, _can_fly, _double_wide);
	}

	const std::vector<int> CombatPathfinder::getHexesInRange(const int _source_hex, const int _range) const {
		if (_source_hex == -1 || _range <= 0)
			throw std::exception("Invalid source hex or range");

		std::unordered_set<int> hexes{ _source_hex };
		std::vector<int> toCheck{ _source_hex };
		std::unordered_set<int> checked{ -1 };

		while (!toCheck.empty()) {
			int hex_id = toCheck.back();
			checked.insert(hex_id);
			toCheck.pop_back();

			if (hex_id == -1)
				continue;

			if (distanceBetweenHexes(hex_id, _source_hex) > _range)
				continue;

			auto adjacent_hexes = getAdjacentHexes(hex_id);

			hexes.insert(std::begin(adjacent_hexes), std::end(adjacent_hexes));
			toCheck.insert(std::end(toCheck), std::begin(adjacent_hexes), std::end(adjacent_hexes));

			for (auto check : checked)
				toCheck.erase(std::remove(std::begin(toCheck), std::end(toCheck), check), std::end(toCheck));
		}

		hexes.erase(-1);
		std::unordered_set<int> to_remove;
		for (auto hex : hexes)
		{
			if (distanceBetweenHexes(_source_hex, hex) > _range) {
				to_remove.insert(hex);
			}
		}

		for (auto rem : to_remove)
			hexes.erase(rem);

		auto result = std::vector<int>(std::begin(hexes), std::end(hexes));
		std::sort(std::begin(result), std::end(result));
		return result;
	}

	const std::vector<int> CombatPathfinder::getWalkableHexesInRange(const int _source_hex, const int _range, const CombatField& _field) const {
		auto hexes = getHexesInRange(_source_hex, _range);
		return getWalkableHexesFromList(hexes, _field);
	}

	const std::vector<int> CombatPathfinder::getWalkableHexesFromList(const std::vector<int>& _hexes, const CombatField& _field) const {
		std::vector<int> walkable(_hexes);

		for (auto hex : _hexes) {
			if (!_field.isHexWalkable(hex))
				walkable.erase(std::remove(std::begin(walkable), std::end(walkable), hex), std::end(walkable));
		}

		return walkable;
	}

	const std::vector<int> CombatPathfinder::getReachableHexesInRange(const int _source_hex, const int _range, const CombatField& _field,
																							const bool _can_fly, const bool _double_wide) const {
		if (_source_hex == -1 || _range <= 0)
			throw std::exception("Invalid source hex or range");
		if (_can_fly || _double_wide)
			throw std::exception("Not implemented yet");

		auto hexes = getWalkableHexesInRange(_source_hex, _range, _field);
		return getReachableHexesFromWalkableHexes(_source_hex, _range, hexes, _field, _can_fly, _double_wide);
	}


	const std::vector<int> CombatPathfinder::getReachableHexesFromWalkableHexes(const int _source_hex, const int _range, 
																										const std::vector<int>& _hexes, const CombatField& _field,
																										const bool _can_fly, const bool _double_wide) const {
		if (_can_fly) {
			if (_double_wide)
				return std::vector<int>(); // todo
			return _hexes; // if can fly and not double_wide, then doesnt need to use pathfinding
		}

		if (_double_wide)
			return std::vector<int>(); // todo

		std::vector<int> reachable;

		for (auto hex : _hexes) {
			auto path = findPath(_source_hex, hex, _field);

			if (!path.empty() && path.size() <= _range)
				reachable.push_back(hex);
		}

		return reachable;
	}

	const int CombatPathfinder::distanceBetweenHexes(const int _source_hex, const int _target_hex) const {
		if (_source_hex == -1 || _target_hex == -1)
			throw std::exception("Invalid source or target hex id");

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

	const std::vector<int> CombatPathfinder::findPath(const int _source_hex, const int _target_hex, const CombatField& _field, const bool _double_wide) const {
		if (_double_wide || _source_hex == -1 || _target_hex == -1)
			throw std::exception("Invalid source or target hex");		
		
		if (!_field.isHexWalkable(_target_hex) || _source_hex == _target_hex)
			return std::vector<int>();

		std::vector<int> path{};

		std::vector<int> to_check{ _source_hex };
		std::vector<int> next_to_check{};
		std::unordered_set<int> checked;
		std::unordered_map<int, int> paths;
		std::unordered_map<int, int> distances;
		bool found = false;
		paths[_source_hex] = -1;
		int dist = 1;

		while (!to_check.empty()) {
			int hex_id = to_check.back();
			to_check.pop_back();
			checked.insert(hex_id);

			if (hex_id == _target_hex) {
				found = true;
				break;
			}

			auto adjacent_hexes = getAdjacentHexesClockwise(hex_id);
			auto walkable = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
			walkable = getWalkableHexesFromList(walkable, _field);

			for (auto hex : walkable) {
				if (paths.find(hex) != std::end(paths)) {
					int distance = hex_id != -1 ? distances[hex_id] + 1 : 999;// ? distanceBetweenHexes(hex_id, _source_hex) : -1;
					int distance2 = paths[hex] != -1 ? distances[hex] : 999; //distanceBetweenHexes(paths[hex], _source_hex) : -1;

					if (distance < distance2)
						paths[hex] = hex_id;
				}
			}

			for (auto check : checked)
				walkable.erase(std::remove(std::begin(walkable), std::end(walkable), check), std::end(walkable));

			checked.insert(std::begin(walkable), std::end(walkable));

			for (auto check : walkable) {
				paths[check] = hex_id;
				distances[check] = dist;
			}

			next_to_check.insert(std::end(next_to_check), std::begin(walkable), std::end(walkable));

			if (to_check.empty()) {
				to_check = next_to_check;
				std::reverse(std::begin(to_check), std::end(to_check));
				next_to_check.clear();
				++dist;
			}
		}

		if (found) {
			int start = _target_hex;

			path.push_back(start);

			while (start != _source_hex) {
				start = paths[start];
				path.push_back(start);
			}

			path.pop_back();
			std::reverse(std::begin(path), std::end(path));
		}

		return path;
	}
}; // HotaMechanics