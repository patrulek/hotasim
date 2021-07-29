#include "combat_ai.h"
#include "combat_hero.h"

#include <cmath>

CombatAI::CombatAI(const CombatManager& _combat_manager)
	: combat_manager(_combat_manager) {}


float CombatAI::calculateUnitAttackFightValueModifier(const CombatUnit& unit) const {
	// if has special abilities
	// if has special spells active
	return 1.0f + 0.05f * unit.calcDiffAtk();
}

float CombatAI::calculateUnitDefenceFightValueModifier(const CombatUnit& unit) const {
	// if has special abilities active
	// if has special spells active
	return 1.0f + 0.05f * unit.calcDiffDef();
}

float CombatAI::calculateUnitFightValueModifier(const CombatUnit& unit) const {
	return sqrt(calculateUnitAttackFightValueModifier(unit) * calculateUnitDefenceFightValueModifier(unit));
}

int CombatAI::calculateStackUnitFightValue(const CombatUnit& unit) const {
	float stack_modifier = calculateUnitFightValueModifier(unit);
	float stack_fight_value = unit.getStackUnitFightValue();

	return static_cast<int>(stack_modifier * stack_fight_value);
}

int CombatAI::calculateHeroFightValue(const CombatHero& hero) const {
	int hero_fight_value = 0;
	
	for (const auto unit : hero.getActiveUnits())
		hero_fight_value += calculateStackUnitFightValue(unit);

	return hero_fight_value;
}

void CombatAI::generatePossibleActions(CombatAI& ai) {
	// check if spellcast possible
	// check if attack possible
	// check if wait / defend / surrender possible and has a sense
	// check if move possible
}

void CombatAI::evaluateAction(CombatAI& ai, CombatAction action, CombatState& state) {

}

// E4580
float CombatAI::multiplierModifier(CombatUnit& activeStack, int side) {
	// tu jakaœ magia, wincyj testów potrzeba
	return 1.0f;
}

// 42770
int CombatAI::calculateSingleUnitValue(CombatUnit& activeStack, int side) {

	int diff_atk = activeStack.calcDiffAtk(); // aka 42130
	int diff_def = activeStack.calcDiffDef(); // aka 422B0
	float current_multiplier = 1.0;

	// modify multiplier by spells
	/*

	if (something)
		// check air shield condition and get multiplier

		if (this->has_shield_spell_active())
			// get_shield_multiplier

			if (this->is_stoned())
				// applicate multiplier (0.5)

				if (this->is_hipnotized())
					// do something (maybe add this value to the other team)

					int side = get_unit_side(); // attacker or defender
	*/

	current_multiplier *= multiplierModifier(activeStack, side); // aka E4580

	float def_multiplier = (diff_def * 0.05 + 1.0);
	float atk_multiplier = (diff_atk * 0.05 * 1.0);


	//if (something)
		// doSomething()

	//	float another_multiplier = (some_value_prob_diff_atk_def) * 0.05 + 1.0;
	//CombatFlags flags = this->get_combat_flags();

	//if (checkIfShooter(flags))
	//	another_multiplier *= 0.5

	//	if (checkSomeSpecificCreature)
			// doSomething()

	//		if (this->is_blessed())
				// do_something
	//		else (this->is_cursed())
				// do_something

	//			another_multiplier *= something(could be another_multiplier);

	float mul_multiplier = def_multiplier * atk_multiplier;
	mul_multiplier = sqrt(mul_multiplier);


	float fight_value = activeStack.unit_template.fightValue;
	float unit_value = fight_value * mul_multiplier;

	//if (this->is_alive()) // probably
		 // ??

	//	return unit_value;
	//	*/
	return unit_value;
}

// 42B80
int CombatAI::calculateUnitValue(CombatUnit& activeStack, int side) {
	if (activeStack.cannotMove(activeStack))
		return -1; // something
	if (activeStack.isArrowTower(activeStack))
		return -1; // something

	if (1 /*?? first param*/ != 1) {
		return -1; // check blind, stone and paralyze
	}

	float singleUnitValue = calculateSingleUnitValue(activeStack, side); // call 42770

	int numberAliveInStack = activeStack.stackNumber;

	//float meleeAttackFightMultiplier = 1.0f; // diffAtk * 0.05 + 1 ; shooterModifier * 0.5 ; shield * 0.5f ; bless/curse */ 2.0;
	// póŸniej jeszcze jakieœ mno¿enie multiplierów; 1.0 * 1.0; a póŸniej jeszcze sqrt robimy
	//float meleeDefenseFightMultiplier = 1.0f; // dunno, maybe
	//float fightValue = activeStack.unitTemplate.fightValue;

	//float multiplier = meleeAttackFightMultiplier * meleeDefenseFightMultiplier * fightValue;

	// if morale or clone, not sure; /=5.0f
	int currentHealth = activeStack.currentStats.hp; // (base hp - helthlost)
	int summedHealth = activeStack.stackNumber * activeStack.unit_template.stats.hp - 0; /* - healthlost*/

	float stackUnitValue = summedHealth * singleUnitValue;
	stackUnitValue /= activeStack.unit_template.stats.hp; // base hp??

	return (int)stackUnitValue; // aka 217F94 
}

// 1EC40 (combat_mgr->get_current_active_side(), activeStack, diffDef, diffAtk, 0 or 1)
int CombatAI::calculateSummedUnitValueForSide(CombatState& state, int side, int minMeleeDiffAtk, int minDiffDef, int something) {
	int sideStacksNumber = combat_manager.getSideStacksNumber(state, side);
	CombatUnit activeStack;// = getUnit(state);

	// for( all units in side )
	int fightValue = calculateUnitValue(activeStack, side);

	return fightValue;
}

// 1F1E0 (activeStack, 0, combat_mgr->get_current_active_side())
void CombatAI::sub1F1E0() {}

// 22130 ()
void CombatAI::calledWhenTacticsState(CombatUnit& unit) {}



// 35B10 (current_active_side, activeStack)
void CombatAI::calculateUnitsFightValues(CombatAI& ai, CombatState& state) {
	CombatUnit activeStack;// = getUnit(state);

	int minMeleeDiffAtk = 0;
	int minDiffDef = 0;

	// iterate over player units
	for (int i = 0; i < 1 /*21 get player unit number*/; ++i) {
		CombatUnit& unit = state.heroes[0].units[i];
		int unitStackNumber = unit.stackNumber;
		bool arrowTower = unit.isArrowTower(unit);
		int diffAtk = unit.calcDiffAtk();
		int diffDef = unit.calcDiffDef();

		if (diffAtk < minMeleeDiffAtk)
			minMeleeDiffAtk = diffAtk;

		if (diffDef < minDiffDef)
			minDiffDef = diffDef;
	}

	// iterate over ai units
	for (int i = 0; i < 1 /*21 get player unit number*/; ++i) {
		CombatUnit& unit = state.heroes[1].units[i];
		int unitStackNumber = unit.stackNumber;
		bool arrowTower = unit.isArrowTower(unit);
		int diffAtk = unit.calcDiffAtk();
		int diffDef = unit.calcDiffDef();

		if (diffAtk < minMeleeDiffAtk)
			minMeleeDiffAtk = diffAtk;

		if (diffDef < minDiffDef)
			minDiffDef = diffDef;
	}

	int player_value_1 = calculateSummedUnitValueForSide(state, 0, minMeleeDiffAtk, minDiffDef, 1); // and atkDiff, defDiff and 1
	int player_value_2 = calculateSummedUnitValueForSide(state, 0, minMeleeDiffAtk, minDiffDef, 0); // and atkDiff, defDiff and 0
	int ai_value_1 = calculateSummedUnitValueForSide(state, 1, minMeleeDiffAtk, minDiffDef, 1); // and atkDiff, defDiff and 1
	int ai_value_2 = calculateSummedUnitValueForSide(state, 1, minMeleeDiffAtk, minDiffDef, 0); // and atkDiff, defDiff and 0

	int ai_fight_value = 1500;
	int player_fight_value = 16870;

	if (ai_value_1 / 2 >= player_value_1)
		; // something
	else {
		if (ai_value_1 >= player_value_2)
			; // something
		else {
			if (ai_value_1 * 5 > player_value_2) {

			}
			else
				; // something = 1 then return
		}
	}
}



// 22100 (activeStack, 0, 0, combat_mgr->get_current_active_side())
void CombatAI::generateRelevantUnitsLists(CombatAI& ai, CombatState& state) {
	CombatUnit activeStack;// = getUnit(state);
	calculateUnitsFightValues(ai, state);


}

// part of 22370
int CombatAI::unitPreconditions(CombatUnit& unit) {
	if (unit.canShoot(unit) || unit.isShootingSiege(unit))
		return 1; // ai.action.type = CombatActionType::MOVE;
	else {
		if (unit.isDoubleWide(unit))
			return 3; // ai.action.type = CombatActionType::CALCULATE;
		else
			return 2; // ai.action.type = CombatActionType::DEFENSE;
	}
}

// 22370 - pick ai action
void CombatAI::pickAction(CombatAI& ai, CombatState& state) {
	ai.action.type = CombatActionType::UNDEFINED;
	CombatUnit activeStack;// = getUnit(state);
	int someValue = unitPreconditions(activeStack);
	ai.action.type = CombatActionType::CALCULATE;

	if (someValue != 1)
		generateRelevantUnitsLists(ai, state);
	else if (combat_manager.isTacticsState(state))
		calledWhenTacticsState(activeStack);
	else
		sub1F1E0();


	// when having lists and fight values, check reachable hexes

	// iterate over possible actions and evaluate them, then pick best action
}