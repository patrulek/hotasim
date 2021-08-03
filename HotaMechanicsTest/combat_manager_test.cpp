#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_action.h"
#include "utils.h"

namespace CombatManagerTest {


	std::vector<int> getCombatFieldTemplate(const int type) {
		// TODO: generate different field templates
		if (type == 1)
			return std::vector<int>{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};


		return std::vector<int>{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
	}
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



	


	TEST(CombatManager, shouldReturnNoAttackNorSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndNoHostileUnitsInRange) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());

		auto hero2 = createHero(createArmy("Peasant", 1));
		CombatField field;

		CombatManager combat_manager(hero, hero2, field, CombatType::NEUTRAL);
		combat_manager.initialize();
		auto& current_state = combat_manager.getCurrentState();
		unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		unit->moveTo(getHexId(8, 1));
		unit->initUnit();
		current_state.field.setTemplate(getCombatFieldTemplate(0));
		//current_state.field.fillHex(getHexId(8, 1), CombatHexOccupation::UNIT);

		auto actions = combat_manager.generateActionsForPlayer(*unit);
		EXPECT_EQ(22, actions.size()); // 20 walking actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(20, walking_actions);

		/// --- ///

		current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::SOLID_OBSTACLE); // this obstacle prevents from going to 2 hexes, so there should be 20 actions now

		actions = combat_manager.generateActionsForPlayer(*unit);
		EXPECT_EQ(20, actions.size()); // 18 walking actions, 1 wait action, 1 defend action

		walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(18, walking_actions);

		/// --- ///

		unit->state.waiting = true; // were changing unit state, so now wait action should be gone
		actions = combat_manager.generateActionsForPlayer(*unit);
		EXPECT_EQ(19, actions.size()); // 18 walking actions, 1 defend action

		walking_actions = 0;
		for (auto action : actions) {
			EXPECT_FALSE(isAttackOrSpellcastAction(action));
			walking_actions += (action.action == CombatActionType::WALK);
		}
		EXPECT_EQ(18, walking_actions);
	}

	TEST(CombatManager, shouldReturnNoSpellcastActionsForPlayerMeleeUnitWhenHeroDoesntHaveSpellbookAndHostileUnitsInRange) {
		auto hero = createHero(createArmy("Peasant", 100));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 100));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager combat_manager(hero, hero2, field, CombatType::NEUTRAL);
		combat_manager.initialize();

		auto& current_state = combat_manager.getCurrentState();
		unit = const_cast<CombatUnit*>(current_state.attacker.getUnits()[0]);
		unit2 = const_cast<CombatUnit*>(current_state.defender.getUnits()[0]);
		current_state.field.setTemplate(getCombatFieldTemplate(0));
		unit->moveTo(getHexId(8, 1));
		unit2->moveTo(getHexId(8, 2));
		current_state.field.fillHex(getHexId(8, 2), CombatHexOccupation::UNIT); // this unit prevents from going to 2 hexes

		auto actions = combat_manager.generateActionsForPlayer(*unit);
		EXPECT_EQ(26, actions.size()); // 18 walking actions, 6 attack actions, 1 wait action, 1 defend action

		int walking_actions = 0;
		int attack_actions = 0;
		for (auto action : actions) {
			walking_actions += (action.action == CombatActionType::WALK);
			attack_actions += (action.action == CombatActionType::ATTACK);
		}
		EXPECT_EQ(18, walking_actions);
		EXPECT_EQ(6, attack_actions);
	}
}