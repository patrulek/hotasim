#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>

#include "structures.h"

namespace HotaMechanics {
	class CombatHex {

	public:
		CombatHex() = default;
		CombatHex(const int16_t _id, const Constants::CombatHexOccupation _occupied_by = Constants::CombatHexOccupation::EMPTY)
			: id(_id), occupied_by(_occupied_by) {}

		// change state -----------------
		void occupyHex(const Constants::CombatHexOccupation type) { occupied_by = type; }
		// ------------------------------

		// check state ------------------
		const bool isWalkable() const;
		// ------------------------------

		// simple getters ---------------
		const int getId() const { return id; }
		const Constants::CombatHexOccupation getOccupation() const { return occupied_by; }
		// ------------------------------

		// util -------------------------
		std::string toString() const;
		// ------------------------------
	private:
		int16_t id{ -1 };
		Constants::CombatHexOccupation occupied_by{ Constants::CombatHexOccupation::EMPTY };
	};

	class CombatField {
	public:
		CombatField() = delete;
		explicit CombatField(const Constants::CombatFieldType _field_type, const Constants::CombatFieldTemplate _field_template = Constants::CombatFieldTemplate::EMPTY);

		// change state ----------------------
		void fillHex(const int16_t _target_hex, const Constants::CombatHexOccupation _occupied_by) {
			if (_target_hex == -1) return;
			hexes[_target_hex].occupyHex(_occupied_by);
		}
		void clearHex(const int16_t _target_hex) {
			if (_target_hex == -1) return;
			fillHex(_target_hex, Constants::CombatHexOccupation::EMPTY);
		}
		void setTemplate(const std::vector<int>& _template);
		// ----------------------------------

		// check state ----------------------
		const bool isHexWalkable(const int16_t _hex_id) const { return _hex_id > -1 && _hex_id < Constants::FIELD_SIZE && hexes[_hex_id].isWalkable(); }
		// ----------------------------------

		// simple getters -------------------
		CombatHex getById(const int16_t _hex_id) const { return hexes[_hex_id]; }
		// ----------------------------------
	private:
		Constants::CombatFieldType combatFieldId{ Constants::CombatFieldType::GRASS };
		Constants::CombatFieldTemplate combatFieldTemplate{ Constants::CombatFieldTemplate::EMPTY };
		std::array<CombatHex, Constants::FIELD_SIZE> hexes;
	};
}; // HotaMechanics