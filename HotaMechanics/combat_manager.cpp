#include "combat_manager.h"

#include "combat_ai.h"
#include "combat_state.h"
#include "combat_unit.h"

void CombatManager::moveUnit(CombatUnit& _unit, int _target_hex) {
	init_state->field.clearHex(_unit.getHex());
	// TODO: check double_wide
	_unit.moveTo(_target_hex);
	init_state->field.fillHex(_target_hex, CombatHexOccupation::UNIT);
	// TODO: check double_wide
}

void CombatManager::placeUnitsBeforeStart() {
	int unit_order = 0;
	for (auto unit : init_state->attacker.getUnits()) {
		int hex = ai->pathfinder->getUnitStartHex(CombatSide::ATTACKER, unit_order++, init_state->attacker.getUnits().size(), unit->isDoubleWide(), combat_type);
		moveUnit(const_cast<CombatUnit&>(*unit), hex);
	}

	unit_order = 0;
	for (auto unit : init_state->defender.getUnits()) {
		int hex = ai->pathfinder->getUnitStartHex(CombatSide::DEFENDER, unit_order++, init_state->defender.getUnits().size(), unit->isDoubleWide(), combat_type);
		moveUnit(const_cast<CombatUnit&>(*unit), hex);
	}
}

void CombatManager::createInitState() {
	CombatHero attacker_(*attacker);
	CombatHero defender_(*defender);
	CombatField field_(*field);

	init_state = std::make_unique<CombatState>(attacker_, defender_, field_);
}

void CombatManager::initialize() {
	if (initialized)
		return;

	createInitState();
	placeUnitsBeforeStart();
	setCurrentState(*init_state);
	initialized = true;
}

std::vector<CombatUnit> CombatManager::getUnitsInRange(CombatSide side, std::vector<int>& hexes) const {
	std::vector<CombatUnit> units_in_range;
	auto hero = side == CombatSide::ATTACKER ? *attacker : *defender;

	for (auto unit : hero.getUnits()) {
		if (!unit->isAlive())
			continue;

		// if unit isnt common unit -> continue

		bool found = std::find(std::begin(hexes), std::end(hexes), unit->getHex()) != std::end(hexes);
		if (found)
			units_in_range.push_back(*unit);
	}

	return units_in_range;
}

const CombatField& CombatManager::getCombatField() const {
	return *field;
}

const CombatAI& CombatManager::getCombatAI() const {
	return *ai;
}

CombatUnit& CombatManager::getActiveStack() {
	//int unitId = current_state->unitOrder[current_state->currentUnit];
	//int side = unitId / 21;
	return const_cast<CombatUnit&>(*current_state->attacker.getUnits()[0]);//const_cast<CombatUnit&>(*current_state->heroes[side].getUnits()[unitId % 21]);
}

CombatManager::CombatManager(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field, const CombatType _combat_type)
	: combat_type(_combat_type)
{
	attacker = std::make_unique<CombatHero>(_attacker);
	defender = std::make_unique<CombatHero>(_defender);
	field = std::make_unique<CombatField>(_field);
	ai = std::make_unique<CombatAI>(*this);
}

CombatManager::~CombatManager() {

}

CombatState CombatManager::nextState() {
	if (!initialized)
		throw std::exception("Combat manager still didnt initialized");

	updateCombat();
	setCombatResult();
	return duplicateCurrentState();
}


void CombatManager::setCombatResult() {
	bool player_alive = current_state->attacker.isAlive();//current_state->heroes[0].isAlive(current_state->heroes[0]);
	bool enemy_alive = current_state->defender.isAlive();//current_state->heroes[0].isAlive(current_state->heroes[1]);

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




CombatAction CombatManager::createWaitAction() const {
	return CombatAction{ CombatActionType::WAIT, -1, -1, true };
}

CombatAction CombatManager::createWalkAction(int hex_id) const {
	return CombatAction{ CombatActionType::WALK, -1, hex_id, true };
}

CombatAction CombatManager::createDefendAction() const {
	return CombatAction{ CombatActionType::DEFENSE, -1, -1, true };
}

CombatAction CombatManager::createSpellCastAction(int spell_id, int unit_id, int hex_id) const {
	throw std::exception("Not implemented yet");
}

CombatAction CombatManager::createAttackAction(int unit_id, int hex_id) const {
	return CombatAction{ CombatActionType::ATTACK, unit_id, hex_id, true };
}

std::vector<CombatAction> CombatManager::generateActionsForPlayer(const CombatUnit& activeStack) {
	if (!activeStack.canMakeAction())
		return std::vector<CombatAction>{};

	std::vector<CombatAction> actions{};

	if (activeStack.canWait())
		actions.push_back(createWaitAction());

	if (activeStack.canDefend())
		actions.push_back(createDefendAction());

	// get reachable hexes;
	auto field = current_state->field;
	auto range_hexes = ai->pathfinder->getHexesInRange(activeStack.getHex(), activeStack.currentStats.spd);
	auto walkable_hexes = ai->pathfinder->getWalkableHexesFromList(range_hexes, field);
	auto reachable_hexes = ai->pathfinder->getReachableHexesFromWalkableHexes(activeStack.getHex(), activeStack.currentStats.spd, walkable_hexes, false, false, field);

	for (auto hex : reachable_hexes)
		actions.push_back(createWalkAction(hex));

	// get attackable enemy units; 
	// if can shoot then only get all enemy units
	auto units_in_range = getUnitsInRange(CombatSide::DEFENDER, range_hexes);

	for (auto unit : units_in_range) {
		auto adjacent_hexes = ai->pathfinder->getAdjacentHexes(unit.getHex());
		auto adjacent_vec = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		auto walkable_adjacent = ai->pathfinder->getWalkableHexesFromList(adjacent_vec, field);
		auto reachable_adjacent = ai->pathfinder->getReachableHexesFromWalkableHexes(activeStack.getHex(), activeStack.currentStats.spd,
			walkable_adjacent, false, false, field);


		// if were staying in one of adjacent hexes to target, add that hex
		bool staying_around = std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), activeStack.getHex()) != std::end(adjacent_hexes);
		if (staying_around)
			reachable_adjacent.push_back(activeStack.getHex());

		for (auto hex : reachable_adjacent) {
			actions.push_back(createAttackAction(unit.getUnitId(), unit.getHex()));
		}
	}

	// get castable spells;
	if (activeStack.canCast())
		throw std::exception("Not implemented yet");

	if (activeStack.canHeroCast())
		throw std::exception("Not implemented yet");

	return actions;
}


std::vector<CombatAction> CombatManager::generateActionsForAI() {
	// check if spellcast possible
	// check if attack possible
	// check if wait / defend / surrender possible and has a sense
	// check if move possible
	return std::vector<CombatAction>{};
}

void CombatManager::evaluateAction(CombatAI& ai, CombatAction action, CombatState& state) {

}