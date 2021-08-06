#include "utils.h"

#include <numeric>

#include "../HotaMechanics/constants.h"
#include "../HotaMechanics/unit_templates.h"
#include "../HotaMechanics/combat_manager.h"

namespace TestUtils {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Constants;


	const bool isAttackOrSpellcastAction(const CombatAction& _action) {
		return _action.action == CombatActionType::ATTACK || _action.action == CombatActionType::SPELLCAST;
	}


	const std::vector<int> range(const int _size) {
		return range(0, _size);
	}

	const std::vector<int> range(const int _min, const int _max) {
		std::vector<int> r(_max - _min);
		std::iota(std::begin(r), std::end(r), _min);
		return r;
	}

	const int getHexId(const int _row, const int _col) {
		return _row * FIELD_COLS + _col;
	}


	const std::vector<UnitStack> createArmy(const std::string tmp1, const int16_t size1,
														 const std::string tmp2, const int16_t size2,
														 const std::string tmp3, const int16_t size3,
														 const std::string tmp4, const int16_t size4,
														 const std::string tmp5, const int16_t size5,
														 const std::string tmp6, const int16_t size6,
														 const std::string tmp7, const int16_t size7) {

		std::vector<UnitStack> army{ UnitStack{ unit_templates()[tmp1], size1 } };
		if (size2 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp2], size2 });
		if (size3 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp3], size3 });
		if (size4 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp4], size4 });
		if (size5 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp5], size5 });
		if (size6 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp6], size6 });
		if (size7 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates()[tmp7], size7 });
		return army;
	}

	CombatHero createHero(const std::vector<UnitStack>& _army, const CombatSide _side, const int _atk, const int _def) {
		Hero tmp;
		tmp.stats.base_stats.atk = _atk;
		tmp.stats.base_stats.def = _def;
		tmp.army = _army;

		return CombatHero(tmp, _side);
	}

	CombatField createField(const CombatFieldType _field_type, const CombatFieldTemplate _field_template) {
		return CombatField(_field_type, _field_template);
	}


	CombatAI& createCombatAI() {
		CombatHero attacker(createHero(createArmy("Imp", 200), CombatSide::ATTACKER));
		CombatHero defender(createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		CombatField field(createField());

		static CombatManager combat_manager(attacker, defender, field, CombatType::NEUTRAL);
		static const CombatAI& ai = combat_manager.getCombatAI();
		return const_cast<CombatAI&>(ai);
	}

	CombatManager* createCombatManager() {
		CombatHero attacker(createHero(createArmy("Imp", 200), CombatSide::ATTACKER));
		CombatHero defender(createHero(createArmy("Peasant", 500), CombatSide::DEFENDER));
		CombatField field(createField());

		CombatManager* combat_manager = new CombatManager(attacker, defender, field, CombatType::NEUTRAL);
		combat_manager->initialize();

		return combat_manager;
	}

	CombatManager* createCombatManager(const CombatHero& _attacker, const CombatHero& _defender, const bool _initialize) {
		CombatField field(createField());

		CombatManager* combat_manager = new CombatManager(_attacker, _defender, field, CombatType::NEUTRAL);
		if( _initialize)
			combat_manager->initialize();

		return combat_manager;
	}

}; //TestUtils