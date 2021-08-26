#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>

#include "structures.h"
#include "utils.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace HotaMechanics {
	class CombatHex {

	public:
		CombatHex() = delete;
		CombatHex(const HexId _id, const Constants::CombatHexOccupation _occupied_by = Constants::CombatHexOccupation::EMPTY)
			: id(_id), occupied_by(_occupied_by), area(calcArea()), walkable(baseWalkable()) {}

		// change state -----------------
		void occupyHex(const Constants::CombatHexOccupation type) { occupied_by = type; }
		// ------------------------------

		// check state ------------------
		const bool isWalkable() const noexcept {
			return walkable && (occupied_by == Constants::CombatHexOccupation::SOFT_OBSTACLE || occupied_by == Constants::CombatHexOccupation::EMPTY);
		}
		// ------------------------------

		// simple getters ---------------
		const HexId getId() const { return id; }
		const HexAreaId getArea() const { return area; }
		const Constants::CombatHexOccupation getOccupation() const { return occupied_by; }
		// ------------------------------

		// util -------------------------
		std::string toString() const;
		// ------------------------------
	private:
		const HexAreaId calcArea() const;
		const bool baseWalkable() const;

		HexId id{ Constants::INVALID_HEX_ID };
		Constants::CombatHexOccupation occupied_by{ Constants::CombatHexOccupation::EMPTY };
		HexAreaId area{ Constants::INVALID_HEX_ID };
		bool walkable{ true };
	};


	class CombatField {
		using HexArray = std::vector<CombatHex>;

	public:
		CombatField() = delete;
		explicit CombatField(const Constants::CombatFieldType _field_type, const Constants::CombatFieldTemplate _field_template = Constants::CombatFieldTemplate::EMPTY);


		// complex getters -------------------
		Hash rehash();
		// -----------------------------------

		// change state ----------------------
		void fillHex(const uint8_t _target_hex, const Constants::CombatHexOccupation _occupied_by) {
			hexes[_target_hex].occupyHex(_occupied_by);

			if (_occupied_by == Constants::CombatHexOccupation::UNIT || _occupied_by == Constants::CombatHexOccupation::SOLID_OBSTACLE
			|| _occupied_by == Constants::CombatHexOccupation::INDESTRUCTIBLE_OBSTACLE)
				occupied.insert(_target_hex);
			else
				occupied.erase(_target_hex);
		}
		void clearHex(const uint8_t _target_hex) {
			hexes[_target_hex].occupyHex(Constants::CombatHexOccupation::EMPTY);
			occupied.erase(_target_hex);
		}
		void setTemplate(const Constants::CombatFieldTemplate _field_template);
		// ----------------------------------

		// check state ----------------------
		const bool isHexWalkable(const uint8_t _hex_id, const bool _ghost_hex = false) const noexcept {
			return hexes[_hex_id].isWalkable() || (_ghost_hex && hexes[_hex_id].getOccupation() == Constants::CombatHexOccupation::UNIT);
		}
		// ----------------------------------

		// simple getters -------------------
		const CombatHex& getById(const HexId _hex_id) const { return hexes[_hex_id]; }
		const HexArray& getHexes() const { return hexes; }
		const Constants::CombatFieldType getType() { return combatFieldId; }
		const Constants::CombatFieldTemplate getTemplate() { return combatFieldTemplate; }
		const Utils::HexSet& getOccupied() const { return occupied; }
		const Hash getHash() const { return hash; }
		// ----------------------------------

		// static creator -------------------
		static CombatField retrieveCombatField(Constants::CombatFieldType _field_type, Constants::CombatFieldTemplate _field_template);
		// ----------------------------------
	private:
		Constants::CombatFieldType combatFieldId{ Constants::CombatFieldType::GRASS };
		Constants::CombatFieldTemplate combatFieldTemplate{ Constants::CombatFieldTemplate::EMPTY };
		HexArray hexes;
		Hash hash{ 0 };
		Utils::HexSet occupied{};

		static std::unordered_map<Constants::CombatFieldTemplate, std::shared_ptr<CombatField>> fields;
	};
}; // HotaMechanics