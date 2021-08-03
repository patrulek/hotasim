#include "gtest/gtest.h"

#include "../HotaSim/combat_simulator.h"

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_action.h"
#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_state.h"
#include "../HotaMechanics/combat_manager.h"

#include <unordered_map>

namespace CombatSimulatorTest {
	std::unordered_map<std::string, Unit> unit_templates{
		/* Simplest units: only walking, no abilities, only melee, etc */
			{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
			{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
			{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
			{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
	};

	bool isAttackOrSpellcastAction(CombatAction action) {
		return action.action == CombatActionType::ATTACK || action.action == CombatActionType::SPELLCAST;
	}


	std::vector<UnitStack> createArmy(const std::string tmp1, const int size1,
		const std::string tmp2 = "", const int size2 = 0,
		const std::string tmp3 = "", const int size3 = 0,
		const std::string tmp4 = "", const int size4 = 0,
		const std::string tmp5 = "", const int size5 = 0,
		const std::string tmp6 = "", const int size6 = 0,
		const std::string tmp7 = "", const int size7 = 0) {

		std::vector<UnitStack> army{ UnitStack{ unit_templates[tmp1], size1 } };
		if (size2 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp2], size2 });
		if (size3 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp3], size3 });
		if (size4 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp4], size4 });
		if (size5 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp5], size5 });
		if (size6 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp6], size6 });
		if (size7 == 0) return army;

		army.emplace_back(UnitStack{ unit_templates[tmp7], size7 });
		return army;
	}

	CombatHero createHero(std::vector<UnitStack>& _army, int _atk = 0, int _def = 0) {
		Hero tmp;
		tmp.setAttack(_atk); tmp.setDefense(_def);
		tmp.setHeroArmy(_army);

		return CombatHero{ tmp };
	}

	TEST(CombatSimulator, shouldInitialCombatStateAlwaysEvaluteToSameNumber) {
		CombatHero attacker = createHero(createArmy("Imp", 100));
		CombatHero defender = createHero(createArmy("Peasant", 200));
		CombatField field;

		CombatManager mgr(attacker, defender, field, CombatType::NEUTRAL);
		CombatSimulator sim(attacker.getTemplate(), defender.getTemplate(), CombatFieldType::GRASS, CombatType::NEUTRAL);
		sim.initialize();
		sim.setCombatManager(mgr);
		
		CombatState init_state{ attacker, defender, field };
		const int64_t expected = 0x0000800080008000;
		EXPECT_EQ(expected, sim.evaluateCombatStateScore(init_state, init_state));


		attacker = createHero(createArmy("Imp", 100, "Goblin", 30));
		defender = createHero(createArmy("Peasant", 200, "Peasant", 50));
		CombatManager mgr2(attacker, defender, field, CombatType::NEUTRAL);
		sim.setCombatManager(mgr2);

		init_state = CombatState{ attacker, defender, field };
		EXPECT_EQ(expected, sim.evaluateCombatStateScore(init_state, init_state));
	}

}