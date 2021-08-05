#include "combat_ai.h"
#include "combat_hero.h"

#include <cmath>

CombatAI::CombatAI(const CombatManager& _combat_manager)
	: combat_manager(_combat_manager) {

	pathfinder = std::make_unique<CombatPathfinder>();
}



std::vector<int> CombatAI::calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero, const CombatField& _field)  {
	std::vector<int> hexes_fight_value(CombatFieldSize::ROWS * CombatFieldSize::COLS, 0);
	int max_fight_value_gain = calculateStackUnitFightValue(activeStack);

	for (auto unit : enemy_hero.getUnits()) {
		int fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, activeStack);

		if (fight_value_gain <= 0)
			continue;

		auto hexes_in_attack_range = pathfinder->getHexesInRange(unit->getHex(), unit->currentStats.spd + 1);
		auto walkable_hexes = pathfinder->getWalkableHexesFromList(hexes_in_attack_range, _field);
		auto reachable_hexes = pathfinder->getReachableHexesFromWalkableHexes(unit->getHex(), unit->currentStats.spd + 1, walkable_hexes, false, false, _field);
		reachable_hexes.push_back(unit->getHex()); // add also unit position

		for (auto hex : reachable_hexes) {
			int hex_fight_value_gain = std::min(max_fight_value_gain, fight_value_gain);
			hexes_fight_value[hex] = std::max(hexes_fight_value[hex], hex_fight_value_gain);
		}
	}

	std::for_each(std::begin(hexes_fight_value), std::end(hexes_fight_value), [](auto& obj) { obj = -obj; });
	hexes_fight_value_gain = hexes_fight_value;
	return hexes_fight_value;
}

int CombatAI::calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const {
	int attacker_base_dmg = attacker.getBaseAverageDmg() * attacker.stackNumber;
	float attacker_bonus_dmg = attacker_base_dmg;

	int attack_advantage = attacker.currentStats.atk - defender.currentStats.def;
	attacker_bonus_dmg *= (((attack_advantage > 0) * 0.05f) + ((attack_advantage < 0) * 0.025f)) * attack_advantage;

	int defender_total_health = defender.stackNumber * defender.currentStats.hp - defender.health_lost;

	// cant do more dmg than total health of defender
	return std::min((int)(attacker_base_dmg + attacker_bonus_dmg), defender_total_health);
}


int CombatAI::calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const {
	CombatUnit attacker_copy(attacker);
	CombatUnit defender_copy(defender);

	int first_attack_dmg = calculateMeleeUnitAverageDamage(attacker_copy, defender_copy);
	defender_copy.applyDamage(first_attack_dmg);

	if (!defender_copy.isAlive() || !defender_copy.canRetaliate())
		return 0;

	int counter_attack_dmg = calculateMeleeUnitAverageDamage(defender_copy, attacker_copy);

	return counter_attack_dmg;
}


int CombatAI::calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender) const {
	CombatUnit attacker_copy(attacker);
	CombatUnit defender_copy(defender);

	int first_attack_dmg = calculateMeleeUnitAverageDamage(attacker_copy, defender_copy);
	defender_copy.applyDamage(first_attack_dmg);

	int counter_attack_dmg = calculateMeleeUnitAverageDamage(defender_copy, attacker_copy);
	attacker_copy.applyDamage(counter_attack_dmg);

	// TODO: when one side fight value > 2 * second side fight value -> then * 1000 for stronger side, and * 100 for weaker side
	int attacker_fight_value_gain = first_attack_dmg * defender_copy.getSingleUnitFightValuePerOneHp();
	int defender_fight_value_gain = counter_attack_dmg * attacker_copy.getSingleUnitFightValuePerOneHp();

	return attacker_fight_value_gain - defender_fight_value_gain;
}

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
	
	for (const auto unit : hero.getUnits())
		hero_fight_value += calculateStackUnitFightValue(*unit);

	return hero_fight_value;
}


int CombatAI::calculateBaseHeroFightValue(const CombatHero& hero) const {
	return calculateHeroFightValue(hero); // TODO: for now its the same, but later need to modify this
}

// because of randomization which cant be mirrored in this project, this function can possibly return more
// than one unit to attack (only if some specified conditions are met; for most cases there will be only one unit)
std::vector<int> CombatAI::chooseUnitToAttack(const CombatUnit& activeStack, const CombatHero& enemy_hero) const {
	if (enemy_hero.getUnits().size() == 1)
		return std::vector<int>{ enemy_hero.getUnits()[0]->getUnitId() }; // todo: to could not always be [0]

	int turns = 999;
	std::vector<int> unit_ids{};
	int fight_value_gain = 0;
	int min_fight_value_gain = 0;
	int max_fight_value_gain = 0;
	int distance = 999;

	for (auto unit : enemy_hero.getUnits()) {
		if (!unit->isAlive())
			continue;

		int unit_distance = pathfinder->distanceBetweenHexes(activeStack.getHex(), unit->getHex()) - 1;
		int unit_turns = unit_distance == 0 ? 1 : std::ceil((float)unit_distance / activeStack.currentStats.spd); // 1 = can attack; todo: check if should be distance - 1
		int unit_fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(activeStack, *unit);
		
		// if no unit picked, pick first
		if (unit_ids.empty()) {
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain / unit_turns;
			min_fight_value_gain = 0.75f * fight_value_gain;
			max_fight_value_gain = fight_value_gain;
			distance = unit_distance;
			continue;
		}

		// if less turns to attack, pick this one
		if (unit_turns < turns) {
			unit_ids.clear();
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain / unit_turns;
			min_fight_value_gain = 0.75f * fight_value_gain;
			max_fight_value_gain = fight_value_gain;
			distance = unit_distance;
			continue;
		}

		// if more turns to attack, check next
		if (unit_turns > turns)
			continue;

		// todo: this is simple randomization example; we'll checking fight_value range from 75%-100% of unit_fight_value_gain
		// if attacking current unit is better, then replace 
		if (unit_fight_value_gain * 0.75f > max_fight_value_gain) { // if cur_min_fv > ch_max_fv -> pick cur
			unit_ids.clear();
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain / unit_turns;
			min_fight_value_gain = 0.75f * fight_value_gain;
			max_fight_value_gain = fight_value_gain;
			distance = unit_distance;
			continue;
		}

		// if attacking chosen unit is better, check next
		if (unit_fight_value_gain < min_fight_value_gain) // if cur_max_fv < ch_min_fv -> pick ch
			continue;

		// fight_value_gain ranges for both units collide, so we need to add current unit to set
		// and update set fight_value_gain ranges
		unit_ids.push_back(unit->getUnitId());
		min_fight_value_gain = std::min(min_fight_value_gain, (int)(0.75f * fight_value_gain));
		max_fight_value_gain = std::max(max_fight_value_gain, fight_value_gain);

		/* 
		* That is how it looks like in H3 code, but as we cant mirror here pseudorandomness
		* we introduce randomness in choosing unit, not calculating its final fight_value_gain
		* 
		// if current unit has more health lost than pick current
		if (unit->health_lost > enemy_hero.getActiveUnits()[unit_ids[0]]->health_lost) {
			unit_ids.clear();
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain;
			distance = unit_distance;
			continue;
		}

		// if current unit has less hp lost then go next
		if (unit->health_lost < enemy_hero.getActiveUnits()[unit_ids[0]]->health_lost)
			continue;

		// if current unit is closer then pick current
		if (unit_distance < distance) {
			unit_ids.clear();
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain;
			distance = unit_distance;
			continue;
		}

		// if current unit is farther then go next
		if (unit_distance > distance)
			continue;

		// if equal then pick current
		unit_ids.push_back(unit->getUnitId());
		turns = unit_turns;
		fight_value_gain = unit_fight_value_gain;
		distance = unit_distance;
		*/
	}

	return unit_ids;

	// if equal number of turns to attack 
	// else pick unit with lesser

	// compare fight_value_gain (tylko �e kurewsko dziwnie, bo por�wnujemy zrandomizowane fight_value / liczb� tur pierwszej jednostki, z tylko zrandomizowanym fight_value drugiej <<bez dzielenia>>, wi�c tak druga zawsze b�dzie wybrana xd lol)

	// if chosen unit	has greater fight_value_gain, then go next
	// else if chosen unit has equal fight_value_gain, pick unit that has more hp lost
	//		if equal hp lost then pick unit that is closer (probably)
	//			if equal distance choose current unit
	// else if chosen unit has less fight_value_gain, pick current unit


	/* fight_value randomization works on TlsGetValue(39 - constant tlsindex for thread), so same action sequences will always result in equal results
		for most combat states this randomization shouldnt matter anyway, because there are more rules directing to choosing proper unit
		randomization return 75-100% of fight value
		// heroes 3 ai multiply calculated value by pseudorandom value in range 0.75-1.00 and then do some more calculations; the result value is a bit lower than calculated, fixed fight value gain/loss
	*/
	// 21D98 - replace units
	// 21DCA - go next
}

int CombatAI::chooseHexToMoveForAttack(const CombatUnit& activeStack, const CombatUnit& target_unit) const {
	auto adjacent_hexes = pathfinder->getAdjacentHexesClockwise(target_unit.getHex());

	int hex = -1;
	int turns = -1;
	int hex_fight_value_gain = 0;
	int distance = -1;

	for (auto adj_hex : adjacent_hexes) {
		if (adj_hex != activeStack.getHex() && !combat_manager.getCurrentState().field.isHexWalkable(adj_hex))
			continue;

		int adj_distance = pathfinder->findPath(activeStack.getHex(), adj_hex, combat_manager.getCurrentState().field).size();
		int adj_turns = adj_distance == 0 ? 1 : std::ceil((float)adj_distance / activeStack.currentStats.spd);
		int adj_hex_fight_value_gain = hexes_fight_value_gain[adj_hex];

		// if didnt choose any yet, check first possible
		if (hex == -1) {
			hex = adj_hex;
			turns = adj_turns;
			hex_fight_value_gain = adj_hex_fight_value_gain;
			distance = adj_distance;
			continue;
		}

		// if current hex is more turns away from us than already chosen, go next
		if (adj_turns > turns)
			continue;

		// if current hex is worse in fight_value_gain terms, go next
		if (adj_hex_fight_value_gain < hex_fight_value_gain)
			continue;
		// if is better then choose this field
		else if (adj_hex_fight_value_gain > hex_fight_value_gain) {
			hex = adj_hex;
			turns = adj_turns;
			hex_fight_value_gain = adj_hex_fight_value_gain;
			distance = adj_distance;
			continue;
		} 

		// if equal, check if we are unit that benefits from longer distances
		if (false /*TODO: check if cavalier or champion*/) {
			if (adj_distance > distance) {
				hex = adj_hex;
				turns = adj_turns;
				distance = adj_distance;
			}

			continue;
		}

		// if not, just choose field that is closer
		if (adj_distance < distance) {
			hex = adj_hex;
			distance = adj_distance;
		}
	}

	return hex;
}


int CombatAI::chooseWalkDistanceFromPath(const CombatUnit& _active_stack, const int _target_hex, const CombatField& _field) const {
	int walk_distance = 0;
	int fight_value_gain = -calculateStackUnitFightValue(_active_stack);
	auto path = pathfinder->findPath(_active_stack.getHex(), _target_hex, _field);
	int range = std::min(path.size(), (size_t)_active_stack.currentStats.spd);

	for (int i = 0; i < range; ++i) {
		if (hexes_fight_value_gain[path[i]] >= fight_value_gain) {
			fight_value_gain = hexes_fight_value_gain[path[i]];
			walk_distance = i + 1;
		}
	}

	return walk_distance;
}

// E4580
//float CombatAI::multiplierModifier(CombatUnit& activeStack, int side) {
//	// tu jaka� magia, wincyj test�w potrzeba
//	return 1.0f;
//}

//// 42770
//int CombatAI::calculateSingleUnitValue(CombatUnit& activeStack, int side) {
//
//	int diff_atk = activeStack.calcDiffAtk(); // aka 42130
//	int diff_def = activeStack.calcDiffDef(); // aka 422B0
//	float current_multiplier = 1.0;
//
//	// modify multiplier by spells
//	/*
//
//	if (something)
//		// check air shield condition and get multiplier
//
//		if (this->has_shield_spell_active())
//			// get_shield_multiplier
//
//			if (this->is_stoned())
//				// applicate multiplier (0.5)
//
//				if (this->is_hipnotized())
//					// do something (maybe add this value to the other team)
//
//					int side = get_unit_side(); // attacker or defender
//	*/
//
//	current_multiplier *= multiplierModifier(activeStack, side); // aka E4580
//
//	float def_multiplier = (diff_def * 0.05 + 1.0);
//	float atk_multiplier = (diff_atk * 0.05 * 1.0);
//
//
//	//if (something)
//		// doSomething()
//
//	//	float another_multiplier = (some_value_prob_diff_atk_def) * 0.05 + 1.0;
//	//CombatFlags flags = this->get_combat_flags();
//
//	//if (checkIfShooter(flags))
//	//	another_multiplier *= 0.5
//
//	//	if (checkSomeSpecificCreature)
//			// doSomething()
//
//	//		if (this->is_blessed())
//				// do_something
//	//		else (this->is_cursed())
//				// do_something
//
//	//			another_multiplier *= something(could be another_multiplier);
//
//	float mul_multiplier = def_multiplier * atk_multiplier;
//	mul_multiplier = sqrt(mul_multiplier);
//
//
//	float fight_value = activeStack.unit_template.fightValue;
//	float unit_value = fight_value * mul_multiplier;
//
//	//if (this->is_alive()) // probably
//		 // ??
//
//	//	return unit_value;
//	//	*/
//	return unit_value;
//}

//// 42B80
//int CombatAI::calculateUnitValue(CombatUnit& activeStack, int side) {
//	if (activeStack.cannotMove(activeStack))
//		return -1; // something
//	if (activeStack.isArrowTower(activeStack))
//		return -1; // something
//
//	if (1 /*?? first param*/ != 1) {
//		return -1; // check blind, stone and paralyze
//	}
//
//	float singleUnitValue = calculateSingleUnitValue(activeStack, side); // call 42770
//
//	int numberAliveInStack = activeStack.stackNumber;
//
//	//float meleeAttackFightMultiplier = 1.0f; // diffAtk * 0.05 + 1 ; shooterModifier * 0.5 ; shield * 0.5f ; bless/curse */ 2.0;
//	// p�niej jeszcze jakie� mno�enie multiplier�w; 1.0 * 1.0; a p�niej jeszcze sqrt robimy
//	//float meleeDefenseFightMultiplier = 1.0f; // dunno, maybe
//	//float fightValue = activeStack.unitTemplate.fightValue;
//
//	//float multiplier = meleeAttackFightMultiplier * meleeDefenseFightMultiplier * fightValue;
//
//	// if morale or clone, not sure; /=5.0f
//	int currentHealth = activeStack.currentStats.hp; // (base hp - helthlost)
//	int summedHealth = activeStack.stackNumber * activeStack.unit_template.stats.hp - 0; /* - healthlost*/
//
//	float stackUnitValue = summedHealth * singleUnitValue;
//	stackUnitValue /= activeStack.unit_template.stats.hp; // base hp??
//
//	return (int)stackUnitValue; // aka 217F94 
//}

// 1EC40 (combat_mgr->get_current_active_side(), activeStack, diffDef, diffAtk, 0 or 1)
//int CombatAI::calculateSummedUnitValueForSide(CombatState& state, int side, int minMeleeDiffAtk, int minDiffDef, int something) {
//	int sideStacksNumber = combat_manager.getSideStacksNumber(state, side);
//	CombatUnit activeStack;// = getUnit(state);
//
//	// for( all units in side )
//	int fightValue = calculateUnitValue(activeStack, side);
//
//	return fightValue;
//}

//// 1F1E0 (activeStack, 0, combat_mgr->get_current_active_side())
//void CombatAI::sub1F1E0() {}
//
//// 22130 ()
//void CombatAI::calledWhenTacticsState(CombatUnit& unit) {}



// 35B10 (current_active_side, activeStack)
//void CombatAI::calculateUnitsFightValues(CombatAI& ai, CombatState& state) {
//	CombatUnit activeStack;// = getUnit(state);
//
//	int minMeleeDiffAtk = 0;
//	int minDiffDef = 0;
//
//	// iterate over player units
//	for (int i = 0; i < 1 /*21 get player unit number*/; ++i) {
//		CombatUnit& unit = const_cast<CombatUnit&>(*state.heroes[0].getUnits()[i]);
//		int unitStackNumber = unit.stackNumber;
//		bool arrowTower = unit.isArrowTower(unit);
//		int diffAtk = unit.calcDiffAtk();
//		int diffDef = unit.calcDiffDef();
//
//		if (diffAtk < minMeleeDiffAtk)
//			minMeleeDiffAtk = diffAtk;
//
//		if (diffDef < minDiffDef)
//			minDiffDef = diffDef;
//	}
//
//	// iterate over ai units
//	for (int i = 0; i < 1 /*21 get player unit number*/; ++i) {
//		CombatUnit& unit = const_cast<CombatUnit&>(*state.heroes[1].getUnits()[i]);
//		int unitStackNumber = unit.stackNumber;
//		bool arrowTower = unit.isArrowTower(unit);
//		int diffAtk = unit.calcDiffAtk();
//		int diffDef = unit.calcDiffDef();
//
//		if (diffAtk < minMeleeDiffAtk)
//			minMeleeDiffAtk = diffAtk;
//
//		if (diffDef < minDiffDef)
//			minDiffDef = diffDef;
//	}
//
//	int player_value_1 = calculateSummedUnitValueForSide(state, 0, minMeleeDiffAtk, minDiffDef, 1); // and atkDiff, defDiff and 1
//	int player_value_2 = calculateSummedUnitValueForSide(state, 0, minMeleeDiffAtk, minDiffDef, 0); // and atkDiff, defDiff and 0
//	int ai_value_1 = calculateSummedUnitValueForSide(state, 1, minMeleeDiffAtk, minDiffDef, 1); // and atkDiff, defDiff and 1
//	int ai_value_2 = calculateSummedUnitValueForSide(state, 1, minMeleeDiffAtk, minDiffDef, 0); // and atkDiff, defDiff and 0
//
//	int ai_fight_value = 1500;
//	int player_fight_value = 16870;
//
//	if (ai_value_1 / 2 >= player_value_1)
//		; // something
//	else {
//		if (ai_value_1 >= player_value_2)
//			; // something
//		else {
//			if (ai_value_1 * 5 > player_value_2) {
//
//			}
//			else
//				; // something = 1 then return
//		}
//	}
//}


//
//// 22100 (activeStack, 0, 0, combat_mgr->get_current_active_side())
//void CombatAI::generateRelevantUnitsLists(CombatAI& ai, CombatState& state) {
//	CombatUnit activeStack;// = getUnit(state);
//	calculateUnitsFightValues(ai, state);
//
//
//}
//
//// part of 22370
//int CombatAI::unitPreconditions(CombatUnit& unit) {
//	if (unit.canShoot(unit) || unit.isShootingSiege(unit))
//		return 1; // ai.action.type = CombatActionType::MOVE;
//	else {
//		if (unit.isDoubleWide())
//			return 3; // ai.action.type = CombatActionType::CALCULATE;
//		else
//			return 2; // ai.action.type = CombatActionType::DEFENSE;
//	}
//}

//// 22370 - pick ai action
//void CombatAI::pickAction(CombatAI& ai, CombatState& state) {
//	ai.action.action = CombatActionType::WALK; // undefined
//	CombatUnit activeStack;// = getUnit(state);
//	int someValue = unitPreconditions(activeStack);
//	ai.action.action = CombatActionType::WALK; // calculate
//
//	if (someValue != 1)
//		generateRelevantUnitsLists(ai, state);
//	else if (combat_manager.isTacticsState(state))
//		calledWhenTacticsState(activeStack);
//	else
//		sub1F1E0();
//
//
//	// when having lists and fight values, check reachable hexes
//
//	// iterate over possible actions and evaluate them, then pick best action
//}