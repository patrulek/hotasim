#include "combat_manager.h"

#include "combat_ai.h"
#include "combat_state.h"
#include "combat_unit.h"


std::vector<int> CombatManager::calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero) const {
	std::vector<int> hexes_fight_value(187, 0);
	int max_fight_value_gain = ai->calculateStackUnitFightValue(activeStack);

	for (auto unit : enemy_hero.getUnits()) {
		int fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, activeStack);
		
		if (fight_value_gain <= 0)
			continue;
		
		auto hexes_in_attack_range = unit->getHexesInAttackRange(current_state->field);
		auto walkable_hexes = current_state->field.getWalkableHexesFromList(hexes_in_attack_range);
		auto reachable_hexes = current_state->field.getReachableHexesFromWalkableHexes(unit->hexId, unit->currentStats.spd + 1, walkable_hexes, false, false);
		reachable_hexes.push_back(unit->hexId); // add also unit position

		for (auto hex : reachable_hexes) {
			int hex_fight_value_gain = std::min(max_fight_value_gain, fight_value_gain);
			hexes_fight_value[hex] = std::max(hexes_fight_value[hex], hex_fight_value_gain);
		}
	}

	std::for_each(std::begin(hexes_fight_value), std::end(hexes_fight_value), [](auto& obj) { obj = -obj; });
	return hexes_fight_value;
}

int CombatManager::calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const {
	int attacker_base_dmg = attacker.getBaseAverageDmg() * attacker.stackNumber;
	float attacker_bonus_dmg = attacker_base_dmg;

	int attack_advantage = attacker.currentStats.atk - defender.currentStats.def;
	attacker_bonus_dmg *= (((attack_advantage > 0) * 0.05f) + ((attack_advantage < 0) * 0.025f)) * attack_advantage;

	int defender_total_health = defender.stackNumber * defender.currentStats.hp - defender.health_lost;

	// cant do more dmg than total health of defender
	return std::min((int)(attacker_base_dmg + attacker_bonus_dmg), defender_total_health);
}


int CombatManager::calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const {
	CombatUnit attacker_copy(attacker);
	CombatUnit defender_copy(defender);

	int first_attack_dmg = calculateMeleeUnitAverageDamage(attacker_copy, defender_copy);
	defender_copy.applyDamage(first_attack_dmg);

	if (!defender_copy.isAlive() || !defender_copy.canRetaliate())
		return 0;

	int counter_attack_dmg = calculateMeleeUnitAverageDamage(defender_copy, attacker_copy);

	return counter_attack_dmg;
}


int CombatManager::calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender) const {
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

std::vector<CombatUnit> CombatManager::getUnitsInRange(CombatSide side, std::vector<int>& hexes) const {
	std::vector<CombatUnit> units_in_range;

	for (auto unit : current_state->heroes[(int)side].getUnits()) {
		if (!unit->isAlive())
			continue;

		// if unit isnt common unit -> continue

		bool found = std::find(std::begin(hexes), std::end(hexes), unit->hexId) != std::end(hexes);
		if (found)
			units_in_range.push_back(*unit);
	}

	return units_in_range;
}

const CombatField& CombatManager::getCombatField() const {
	return current_state->field;
}

const CombatAI& CombatManager::getCombatAI() const {
	return *ai;
}

CombatUnit& CombatManager::getActiveStack() {
	int unitId = current_state->unitOrder[current_state->currentUnit];
	int side = unitId / 21;
	return const_cast<CombatUnit&>(*current_state->heroes[side].getUnits()[unitId % 21]);
}

CombatManager::CombatManager(const CombatHero& attacker, const CombatHero& defender, const CombatField& field)
	: init_state(CombatState(attacker, defender, field))
{
	ai = std::make_unique<CombatAI>(*this);
	current_state = std::make_unique<CombatState>(init_state);
}

CombatManager::~CombatManager() {

}

CombatState CombatManager::nextState() {
	updateCombat();
	setCombatResult();
	return duplicateCurrentState();
}


void CombatManager::setCombatResult() {
	bool player_alive = current_state->heroes[0].isAlive(current_state->heroes[0]);
	bool enemy_alive = current_state->heroes[0].isAlive(current_state->heroes[1]);

	if (player_alive && enemy_alive) current_state->result = CombatResult::IN_PROGRESS;
	else if (player_alive)				current_state->result = CombatResult::PLAYER;
	else if (enemy_alive)				current_state->result = CombatResult::ENEMY;
	else										current_state->result = CombatResult::DRAW;
}

void CombatManager::orderUnitsInTurn()
{
	for (int i = 0; i < 84; ++i)
		current_state->unitOrder[i] = -1;
}

void CombatManager::preTurnUpdate()
{
	// reactivate spellbook
	// decrease spell active on units
	// place units in order
	orderUnitsInTurn();
}

void CombatManager::updateTurn()
{
	if (isNewTurn()) {
		preTurnUpdate();
	}
}

void CombatManager::updateCombat()
{
	if (isNewCombat())
		preCombatUpdate();

	updateTurn();
}

bool CombatManager::isNewTurn()
{
	return true; // check if unitid > unitsDone;
}

bool CombatManager::isNewCombat()
{
	return current_state->result == CombatResult::NOT_STARTED;
}
