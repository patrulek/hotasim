#pragma once


#include <vector>
#include <cmath>
#include <algorithm>
#include <array>

#include "structures.h"
#include <iostream>
#include <unordered_map>

namespace HotaMechanics {
	class CombatHex {

	public:
		CombatHex() = default;
		CombatHex(const int16_t _id, const Constants::CombatHexOccupation _occupied_by = Constants::CombatHexOccupation::EMPTY)
			: id(_id), occupied_by(_occupied_by), area(calcArea()), walkable(baseWalkable()) {}
		CombatHex(const CombatHex& _obj) = default;
		CombatHex(CombatHex&& _obj) = default;
		CombatHex& operator=(const CombatHex& _obj) = default;

		// change state -----------------
		void occupyHex(const Constants::CombatHexOccupation type) { occupied_by = type; }
		// ------------------------------

		// check state ------------------
		const bool isWalkable() const;
		// ------------------------------

		// simple getters ---------------
		const int16_t getId() const { return id; }
		const int16_t getArea() const { return area; }
		const Constants::CombatHexOccupation getOccupation() const { return occupied_by; }
		// ------------------------------

		// util -------------------------
		std::string toString() const;
		// ------------------------------
	private:
		const int16_t calcArea() const;
		const bool baseWalkable() const;

		int16_t id{ Constants::INVALID_HEX_ID };
		Constants::CombatHexOccupation occupied_by{ Constants::CombatHexOccupation::EMPTY };
		int16_t area;
		bool walkable;
	};
	
	using HexArray = std::array<CombatHex, Constants::FIELD_SIZE + 1>;

	struct StaticHexArray {
		std::vector<CombatHex> hexes;

		StaticHexArray() {
			hexes.reserve(Constants::FIELD_SIZE + 1);
			for (int hex = 0; hex < Constants::FIELD_SIZE + 1; ++hex)
				hexes.emplace_back(hex);
		}

	};


	class CombatField {

	public:
		CombatField() = delete;
		explicit CombatField(const Constants::CombatFieldType _field_type, const Constants::CombatFieldTemplate _field_template = Constants::CombatFieldTemplate::EMPTY);
		CombatField(const CombatField& _obj) = default; 
		CombatField(CombatField&& _obj) = default;
		/*{


			std::cout << "copy " << std::endl;
		}*/
		CombatField& operator=(const CombatField& _field) {
			combatFieldId = _field.combatFieldId;
			combatFieldTemplate = _field.combatFieldTemplate;
			for (auto& hex : _field.hexes)
				hexes[hex.getId()].occupyHex(hex.getOccupation());

			return *this;
		}

		// complex getters -------------------
		const int64_t getHash() const;
		// -----------------------------------

		// change state ----------------------
		void fillHex(const int16_t _target_hex, const Constants::CombatHexOccupation _occupied_by) {
			hexes[_target_hex].occupyHex(_occupied_by);
		}
		void clearHex(const int16_t _target_hex) {
			fillHex(_target_hex, Constants::CombatHexOccupation::EMPTY);


		}
		void setTemplate(const Constants::CombatFieldTemplate _field_template);
		void rehash();
		// ----------------------------------

		// check state ----------------------
		const bool isHexWalkable(const int16_t _hex_id, const bool _ghost_hex = false) const {
			return hexes[_hex_id].isWalkable() || (_ghost_hex && hexes[_hex_id].getOccupation() == Constants::CombatHexOccupation::UNIT);
		}
		// ----------------------------------

		// simple getters -------------------
		const CombatHex& getById(const int16_t _hex_id) const { return hexes[_hex_id]; }
		const HexArray& getHexes() const { return hexes; }
		const Constants::CombatFieldType getType() { return combatFieldId; }
		const Constants::CombatFieldTemplate getTemplate() { return combatFieldTemplate; }
		// ----------------------------------

		// static creator -------------------
		static CombatField retrieveCombatField(Constants::CombatFieldType _field_type, Constants::CombatFieldTemplate _field_template);
		// ----------------------------------
	private:
		Constants::CombatFieldType combatFieldId{ Constants::CombatFieldType::GRASS };
		Constants::CombatFieldTemplate combatFieldTemplate{ Constants::CombatFieldTemplate::EMPTY };
		HexArray hexes;
		int64_t hash{ 0 };

		static std::unordered_map<Constants::CombatFieldTemplate, CombatField*> fields;
	};
}; // HotaMechanics