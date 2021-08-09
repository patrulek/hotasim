#include "combat_field.h"

#include "utils.h"

#include <functional>

namespace HotaMechanics {
	using namespace Constants;

	const int64_t CombatField::getHash() const {
		int64_t hash{ 0 };
		std::array<int16_t, 9> areas; areas.fill(0);

		for (auto& hex : hexes)
			areas[hex.getArea()] += !hex.isWalkable();
		
		for (int area_id = 0; area_id < areas.size(); ++area_id)
			hash |= ((int64_t)areas[area_id] << area_id);

		return std::hash<int64_t>{}(hash);
	}

	const bool CombatHex::baseWalkable() const {
		return !(id % Constants::FIELD_COLS == 0) || (id % Constants::FIELD_COLS == Constants::FIELD_COLS - 1);
	}

	const int16_t CombatHex::calcArea() const {
		const int row = id / FIELD_COLS;
		const int col = id / FIELD_ROWS;

		const int base = (col > 5) + (col > 10);
		const int shift = 3 * ((row > 3) + (row > 6));

		return base + shift;
	}

	std::string CombatHex::toString() const {
		return "Hex(" + std::to_string(id / FIELD_COLS) + ", " + std::to_string(id % FIELD_COLS) + ")";
	}

	const bool CombatHex::isWalkable() const {
		return walkable && (occupied_by == CombatHexOccupation::SOFT_OBSTACLE || occupied_by == CombatHexOccupation::EMPTY);
	}

	HexArray CombatField::initializeHexes() {
		std::vector<CombatHex> vexes;
		for (int hex = 0; hex < FIELD_SIZE + 1; ++hex)
			vexes.push_back(CombatHex(hex));)
	}

	CombatField::CombatField(const CombatFieldType _field_type, const CombatFieldTemplate _field_template)
		: combatFieldId(_field_type), combatFieldTemplate(_field_template), hexes(initializeHexes()) {
		auto field_template = Utils::getCombatFieldTemplate(_field_template);
		setTemplate(field_template);
	}

	void CombatField::setTemplate(const std::vector<int>& _template) {
		if (_template.size() != FIELD_SIZE)
			throw std::exception("Wrong template size");

		for (int hex = 0; hex < FIELD_SIZE; ++hex) {
			const auto occupation = _template[hex] == 1 ? CombatHexOccupation::SOLID_OBSTACLE : CombatHexOccupation::EMPTY;
			hexes[hex].occupyHex(occupation);
		}
	}
}; // HotaMechanics
