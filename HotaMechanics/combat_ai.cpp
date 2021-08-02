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
		hero_fight_value += calculateStackUnitFightValue(*unit);

	return hero_fight_value;
}


CombatAction CombatAI::createWaitAction() const {
	return CombatAction{ CombatActionType::WAIT, -1, -1, true };
}

CombatAction CombatAI::createWalkAction(int hex_id) const {
	return CombatAction{ CombatActionType::WALK, -1, hex_id, true };
}

CombatAction CombatAI::createDefendAction() const {
	return CombatAction{ CombatActionType::DEFENSE, -1, -1, true };
}

CombatAction CombatAI::createSpellCastAction(int spell_id, int unit_id, int hex_id) const {
	throw std::exception("Not implemented yet");
}

CombatAction CombatAI::createAttackAction(int unit_id, int hex_id) const {
	return CombatAction{ CombatActionType::ATTACK, unit_id, hex_id, true };
}

std::vector<CombatAction> CombatAI::generateActionsForPlayer(const CombatUnit& activeStack) const {
	if (!activeStack.canMakeAction())
		return std::vector<CombatAction>{};

	std::vector<CombatAction> actions{};

	if (activeStack.canWait())
		actions.push_back(createWaitAction());

	if (activeStack.canDefend())
		actions.push_back(createDefendAction());

	// get reachable hexes;
	auto field = combat_manager.getCombatField();
	auto range_hexes = field.getHexesInRange(activeStack.hexId, activeStack.currentStats.spd);
	auto walkable_hexes = field.getWalkableHexesFromList(range_hexes);
	auto reachable_hexes = field.getReachableHexesFromWalkableHexes(activeStack.hexId, activeStack.currentStats.spd, walkable_hexes, false, false);

	for (auto hex : reachable_hexes)
		actions.push_back(createWalkAction(hex));
	
	// get attackable enemy units; 
	// if can shoot then only get all enemy units
	auto units_in_range = combat_manager.getUnitsInRange(CombatSide::DEFENDER, range_hexes);
	 
	for (auto unit : units_in_range) {
		auto adjacent_hexes = field.getById(unit.hexId).getAdjacentHexes();
		auto adjacent_vec = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		auto walkable_adjacent = field.getWalkableHexesFromList(adjacent_vec);
		auto reachable_adjacent = field.getReachableHexesFromWalkableHexes(activeStack.hexId, activeStack.currentStats.spd,
																								 walkable_adjacent, false, false);

		
		// if were staying in one of adjacent hexes to target, add that hex
		bool staying_around = std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), activeStack.hexId) != std::end(adjacent_hexes);
		if (staying_around)
			reachable_adjacent.push_back(activeStack.hexId);

		for (auto hex : reachable_adjacent) {
			actions.push_back(createAttackAction(unit.getUnitId(), unit.hexId));
		}
	}
	
	// get castable spells;
	if (activeStack.canCast())
		throw std::exception("Not implemented yet");

	if (activeStack.canHeroCast())
		throw std::exception("Not implemented yet");

	return actions;
}


int CombatAI::chooseUnitToAttack(const CombatUnit& activeStack, const CombatHero& enemy_hero) const {
	if (enemy_hero.getActiveUnits().size() == 1)
		return enemy_hero.getActiveUnits()[0]->getUnitId();

	throw std::exception("Not implemented yet");

	// iterate over units

	// if first unit, choose it

	// if equal number of turns to attack 
	// else pick unit with lesser

	// compare fight_value_gain (tylko ¿e kurewsko dziwnie, bo porównujemy zrandomizowane fight_value / liczbê tur pierwszej jednostki, z tylko zrandomizowanym fight_value drugiej <<bez dzielenia>>, wiêc tak druga zawsze bêdzie wybrana xd lol)

	// if chosen unit	has greater fight_value_gain, then go next
	// else if chosen unit has equal fight_value_gain, pick unit that has more hp lost
	//		if equal hp lost then pick unit that is closer (probably)
	//			if equal distance choose current unit
	// else if chosen unit has less fight_value_gain, pick current unit

	// fight_value_gain is randomized and divided by 
	// 21D98 - replace units
	// 21DCA - go next
}

float CombatAI::randomizeChoice(int fight_value_gain) const {
	/* randomization works on TlsGetValue(39 - constant tlsindex for thread), so same action sequences will always result in equal results 
		for most combat states this randomization shouldnt matter anyway, because there are more rules directing to choosing proper unit
	*/

	return 1.0f * fight_value_gain; // todo: implement this; more info in declaration
}

int CombatAI::chooseHexToMoveForAttack(const CombatUnit& activeStack, const CombatUnit& target_unit) const {
	auto adjacent_hexes = combat_manager.getCombatField().getById(target_unit.hexId).getAdjacentHexesClockwise();
	auto hexes_fight_value = combat_manager.calculateFightValueAdvantageOnHexes(activeStack, *target_unit.hero);

	int hex = -1;
	int turns = -1;
	int hex_fight_value_gain = 0;
	int distance = -1;

	for (auto adj_hex : adjacent_hexes) {
		if (!combat_manager.getCombatField().getById(adj_hex).isWalkable())
			continue;

		int adj_distance = combat_manager.getCombatField().getById(adj_hex).distanceToHex(activeStack.hexId);
		int adj_turns = std::ceil(adj_distance / activeStack.currentStats.spd);
		int adj_hex_fight_value_gain = hexes_fight_value[adj_hex];

		// if didnt choose any yet, check first possible
		if (hex == -1) {
			hex = adj_hex;
			turns = adj_turns;
			hex_fight_value_gain = adj_hex_fight_value_gain;
			continue;
		}

		// if current hex is more turns away from us than already chosen, go next
		if (adj_turns > turns)
			continue;

		// if current hex is worse in fight_value_gain terms, go next
		if (adj_hex_fight_value_gain < hex_fight_value_gain)
			continue;
		// if is better than choose this field
		else if (adj_hex_fight_value_gain > hex_fight_value_gain) {
			hex = adj_hex;
			turns = adj_turns;
			hex_fight_value_gain = adj_hex_fight_value_gain;
			continue;
		} 

		// if equal, check if we are unit that benefits from longer distances
		if (false /*TODO: check if cavalier or champion*/) {
			if (adj_distance > distance) {
				hex = adj_hex;
				turns = adj_turns;
			}

			continue;
		}

		// if not, just choose field that is closer
		if (adj_distance < distance) {
			hex = adj_hex;
			turns = adj_turns;
		}
	}

	return hex;
}

std::vector<CombatAction> CombatAI::generateActionsForAI() {
	// check if spellcast possible
	// check if attack possible
	// check if wait / defend / surrender possible and has a sense
	// check if move possible
	return std::vector<CombatAction>{};
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
	ai.action.action = CombatActionType::WALK; // undefined
	CombatUnit activeStack;// = getUnit(state);
	int someValue = unitPreconditions(activeStack);
	ai.action.action = CombatActionType::WALK; // calculate

	if (someValue != 1)
		generateRelevantUnitsLists(ai, state);
	else if (combat_manager.isTacticsState(state))
		calledWhenTacticsState(activeStack);
	else
		sub1F1E0();


	// when having lists and fight values, check reachable hexes

	// iterate over possible actions and evaluate them, then pick best action
}