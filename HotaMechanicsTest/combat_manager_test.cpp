#include "gtest/gtest.h"

#include <unordered_map>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "utils.h"

namespace CombatManagerTest {
	std::unordered_map<std::string, Unit> unit_templates{
		/* Simplest units: only walking, no abilities, only melee, etc */
			{"Peasant", { 15, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
			{ "Gremlin", { 55, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
			{ "Goblin", { 60, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
			{ "Imp", { 50, {2, 3, 1, 2, 0, 0, 5, 4, 0}, SpellBook{}, "Imp" } }
	};

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



	TEST(CombatAI, shouldReturnCorrectMeleeUnitDmgWithoutAnyModifiers) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		// 200 imps should do 315 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(315, mgr.calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 475 dmg on average to 200 imps without any attack/defense and/or spells modifiers
		EXPECT_EQ(475, mgr.calculateMeleeUnitAverageDamage(*unit2, *unit));

		hero = createHero(createArmy("Imp", 100));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));
		// 100 imps should do 157 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(157, mgr.calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 400 dmg on average to 100 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(400, mgr.calculateMeleeUnitAverageDamage(*unit2, *unit));

		hero = createHero(createArmy("Imp", 50));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));
		// 50 imps should do 78 dmg on average to 500 peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(78, mgr.calculateMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should do 200 dmg on average to 50 imps (killing all) without any attack/defense and/or spells modifiers
		EXPECT_EQ(200, mgr.calculateMeleeUnitAverageDamage(*unit2, *unit));

		hero = createHero(createArmy("Goblin", 30));
		unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		// 30 goblins should do 51 dmg on average to peasants without any attack/defense and/or spells modifiers
		EXPECT_EQ(51, mgr.calculateMeleeUnitAverageDamage(*unit, *unit2));
	}


	TEST(CombatAI, shouldReturnZeroForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsKilledOrCantRetaliate) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 100));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->state.retaliated = false;
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		// 200 imps should kill all peasants, so 0 dmg in counterattack
		EXPECT_EQ(0, mgr.calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));


		hero2 = createHero(createArmy("Peasant", 500));
		unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->state.retaliated = true;
		unit2->moveTo(getHexId(8, 2));

		// 200 imps will not kill all peasants, but they already retaliated so 0 dmg
		EXPECT_EQ(0, mgr.calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));
	}


	TEST(CombatAI, shouldReturnCorrectValueForCounterattackMeleeUnitDmgWithoutAnyModifiersWhenUnitIsNotKilledAndCanRetaliate) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->state.retaliated = false;
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		// 200 imps should kill 315 peasants, so 185 peasants should retaliate 175 dmg to imps
		EXPECT_EQ(175, mgr.calculateCounterAttackMeleeUnitAverageDamage(*unit, *unit2));

		// 500 peasants should kill 118 imps, so 82 imps should retaliate 129 to peasants
		EXPECT_EQ(129, mgr.calculateCounterAttackMeleeUnitAverageDamage(*unit2, *unit));
	}


	TEST(CombatAI, shouldReturnCorrectFightValueGainForMeleeUnitAttackWithoutAnyModifiersWhenSimilarArmyStrength) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		// fight value gain when 200 imps attack 500 peasants first
		EXPECT_EQ(2538, mgr.calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, *unit2));

		// fight value gain when 500 peasants attack 200 imps first
		EXPECT_EQ(4002, mgr.calculateFightValueAdvantageAfterMeleeUnitAttack(*unit2, *unit));
	}

	TEST(CombatAI, shouldReturnOnlyZeroFightValueGainOnHexesForMeleeUnitWhenAttackerWeakerThanDefender) {
		auto hero = createHero(createArmy("Imp", 50));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(8, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(8, 2));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		std::vector<int> expected(187, 0);
		EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(*unit2, hero));
	}

	TEST(CombatAI, shouldReturnCorrectFightValueGainOnHexesForMeleeUnitWhenAttackerLittleStrongerThanDefender) {
		auto hero = createHero(createArmy("Imp", 200));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(5, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(5, 15));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		int fv = -2538;
		std::vector<int> expected{
			0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(*unit2, hero));
	}


	TEST(CombatAI, DISABLED_shouldReturnMaximumFightValueGainOnHexesForMeleeUnitWhenAttackerIncrediblyStrongerThanDefender) {
		auto hero = createHero(createArmy("Imp", 301));
		auto unit = const_cast<CombatUnit*>(hero.getUnits().front());
		unit->applyHeroStats();
		unit->initUnit();
		unit->moveTo(getHexId(5, 1));

		auto hero2 = createHero(createArmy("Peasant", 500));
		auto unit2 = const_cast<CombatUnit*>(hero2.getUnits().front());
		unit2->applyHeroStats();
		unit2->initUnit();
		unit2->moveTo(getHexId(5, 15));

		CombatField field;
		CombatManager mgr(hero, hero2, field, CombatType::NEUTRAL);

		int fv = -7500; // TODO: implement case when attacker fight value > 2 * defender fight value; check if there isnt case for 5 * also
		std::vector<int> expected{
			0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, fv, fv, fv, fv, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		EXPECT_EQ(expected, mgr.calculateFightValueAdvantageOnHexes(*unit2, hero));
	}
}