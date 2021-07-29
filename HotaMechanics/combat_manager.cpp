#include "combat_manager.h"

#include "combat_ai.h"
#include "combat_state.h"
#include "combat_unit.h"

const CombatAI& CombatManager::getCombatAI() const {
	return *ai;
}

CombatUnit& CombatManager::getActiveStack() {
	int unitId = current_state->unitOrder[current_state->currentUnit];
	int side = unitId / 21;
	return current_state->heroes[side].units[unitId % 21];
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
