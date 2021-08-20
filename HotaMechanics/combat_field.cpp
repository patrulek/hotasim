#include "combat_field.h"

#include "utils.h"

#include <functional>
#include <initializer_list>
#include <unordered_map>

namespace HotaMechanics {
	using namespace Constants;
	using namespace Utils;

	Hash CombatField::rehash() {
		hash = 0;
		for (auto& hex : occupied)
			hash ^= std::hash<Hash>{}(hex);

		return hash;
	}

	const bool CombatHex::baseWalkable() const {
		return !(id % Constants::FIELD_COLS == 0 || id % Constants::FIELD_COLS == Constants::FIELD_COLS - 1);
	}

	const HexAreaId CombatHex::calcArea() const {
		const uint8_t row = id / FIELD_COLS;
		const uint8_t col = id % FIELD_COLS;

		const uint8_t base = (col > 5) + (col > 10);
		const uint8_t shift = 3 * ((row > 3) + (row > 6));

		return base + shift;
	}

	std::string CombatHex::toString() const {
		return "Hex(" + std::to_string(id / FIELD_COLS) + ", " + std::to_string(id % FIELD_COLS) + ")";
	}

	CombatField::CombatField(const CombatFieldType _field_type, const CombatFieldTemplate _field_template)
		: combatFieldId(_field_type), combatFieldTemplate(_field_template) {
		if (fields.find(_field_template) != std::end(fields)) {
			combatFieldTemplate = _field_template;
			combatFieldId = _field_type;
			hexes = fields[_field_template]->hexes;
			hash = fields[_field_template]->hash;
			occupied = fields[_field_template]->occupied;
			return;
		}

		occupied.rehash(64);
		hexes.reserve(FIELD_SIZE + 1);
		setTemplate(_field_template);
		rehash();

		if (fields.find(_field_template) == std::end(fields))
			fields[_field_template] = std::make_shared<CombatField>(std::move(CombatField(*this)));
	}

	void CombatField::setTemplate(const CombatFieldTemplate _field_template) {
		const std::vector<HexId>& _template = Utils::getCombatFieldTemplate(_field_template);
		occupied.clear();
		hexes.clear();

		if (_template.size() != FIELD_SIZE)
			throw std::exception("Wrong template size");

		combatFieldTemplate = _field_template;

		for (HexId hex = 0; hex < FIELD_SIZE; ++hex) {
			const auto occupation = _template[hex] == 1 ? CombatHexOccupation::SOLID_OBSTACLE : CombatHexOccupation::EMPTY;
			hexes.emplace_back(hex, occupation);
			if (_template[hex] == 1)
				occupied.insert(hex);
		}

		hexes.emplace_back(INVALID_HEX_ID);
	}

	CombatField CombatField::retrieveCombatField(CombatFieldType _field_type, CombatFieldTemplate _field_template) {
		if (fields.find(_field_template) != std::end(fields))
			return *fields[_field_template];
		return CombatField(_field_type, _field_template);
	}

	std::unordered_map<CombatFieldTemplate, std::shared_ptr<CombatField>> CombatField::fields;
}; // HotaMechanics
