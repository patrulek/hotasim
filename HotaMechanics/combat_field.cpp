#include "combat_field.h"

#include "utils.h"

#include <functional>
#include <initializer_list>
#include <unordered_map>

namespace HotaMechanics {
	using namespace Constants;
	using namespace Utils;

	int64_t CombatField::rehash() {
		hash = 0;
		for (auto& hex : hexes)
			hash += (hex.isWalkable() * ((hex.getId() % FIELD_COLS) << (4 * hex.getId() / FIELD_COLS)));

		hash = std::hash<int64_t>{}(hash);
		return hash;
	}

	const int64_t CombatField::getHash() const {
		return hash;
	}

	const bool CombatHex::baseWalkable() const {
		return !(id % Constants::FIELD_COLS == 0 || id % Constants::FIELD_COLS == Constants::FIELD_COLS - 1);
	}

	const int16_t CombatHex::calcArea() const {
		const int row = id / FIELD_COLS;
		const int col = id % FIELD_COLS;

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

	CombatField::CombatField(const CombatFieldType _field_type, const CombatFieldTemplate _field_template)
		: combatFieldId(_field_type), combatFieldTemplate(_field_template) {
		if (fields.find(_field_template) != std::end(fields)) {
			combatFieldTemplate = _field_template;
			combatFieldId = _field_type;
			hexes = fields[_field_template]->hexes;
			return;
		}

		setTemplate(_field_template);
	}

	void CombatField::setTemplate(const CombatFieldTemplate _field_template) {
		const std::vector<int>& _template = Utils::getCombatFieldTemplate(_field_template);

		if (_template.size() != FIELD_SIZE)
			throw std::exception("Wrong template size");

		combatFieldTemplate = _field_template;

		for (int hex = 0; hex < FIELD_SIZE; ++hex) {
			const auto occupation = _template[hex] == 1 ? CombatHexOccupation::SOLID_OBSTACLE : CombatHexOccupation::EMPTY;
			hexes[hex] = CombatHex(hex, occupation);
		}

		hexes[INVALID_HEX_ID] = CombatHex(INVALID_HEX_ID);

		if (fields.find(_field_template) == std::end(fields))
			fields[_field_template] = new CombatField(*this);
		//rehash();
	}

	CombatField CombatField::retrieveCombatField(CombatFieldType _field_type, CombatFieldTemplate _field_template) {
		if (fields.find(_field_template) != std::end(fields))
			return *fields[_field_template];
		return CombatField(_field_type, _field_template);
	}

	std::unordered_map<CombatFieldTemplate, CombatField*> CombatField::fields;
}; // HotaMechanics
