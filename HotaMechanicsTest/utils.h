#pragma once

#include <memory>

#include "gmock/gmock.h"

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_manager.h"

namespace TestUtils {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Constants;

	const bool isAttackOrSpellcastAction(const CombatAction& _action);
	const std::vector<size_t> range(const size_t _size);
	const std::vector<size_t> range(const size_t _min, const size_t _max);
	const std::vector<UnitStack> createArmy(const std::string tmp1, const int16_t size1,
		const std::string tmp2 = "", const int16_t size2 = 0,
		const std::string tmp3 = "", const int16_t size3 = 0,
		const std::string tmp4 = "", const int16_t size4 = 0,
		const std::string tmp5 = "", const int16_t size5 = 0,
		const std::string tmp6 = "", const int16_t size6 = 0,
		const std::string tmp7 = "", const int16_t size7 = 0);
	CombatHero createHero(const std::vector<UnitStack>& _army, const CombatSide _side = CombatSide::ATTACKER, const int _atk = 0, const int _def = 0);
	CombatField createField(const CombatFieldType _field_type = CombatFieldType::GRASS, const CombatFieldTemplate _field_template = CombatFieldTemplate::EMPTY);
	CombatAI& createCombatAI();
	CombatManager* createCombatManager();
	CombatManager* createCombatManager(const CombatHero& _attacker, const CombatHero& _defender, const bool _initialize = true);
	const uint8_t getHexId(const int _row, const int _col);

}; // TestUtils;