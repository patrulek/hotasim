#include "combat_field.h"

#include "utils.h"

namespace HotaMechanics {
	using namespace Constants;

	std::string CombatHex::toString() const {
		return "Hex(" + std::to_string(id / FIELD_COLS) + ", " + std::to_string(id % FIELD_COLS) + ")";
	}

	const bool CombatHex::isWalkable() const {
		bool first_col = id % FIELD_COLS == 0;
		bool last_col = id % FIELD_COLS == FIELD_COLS - 1;

		return (occupied_by == CombatHexOccupation::SOFT_OBSTACLE || occupied_by == CombatHexOccupation::EMPTY)
			&& !(first_col || last_col);
	}



	CombatField::CombatField(const CombatFieldType _field_type, const CombatFieldTemplate _field_template)
		: combatFieldId(_field_type) {
		auto field_template = Utils::getCombatFieldTemplate(_field_template);
		setTemplate(field_template);
	}

	void CombatField::setTemplate(const std::vector<int>& _template) {
		if (_template.size() != FIELD_SIZE)
			throw std::exception("Wrong template size");

		for (int hex = 0; hex < FIELD_SIZE; ++hex) {
			const auto occupation = _template[hex] == 1 ? CombatHexOccupation::SOLID_OBSTACLE : CombatHexOccupation::EMPTY;
			hexes[hex] = CombatHex(hex, occupation);
		}
	}
}; // HotaMechanics
