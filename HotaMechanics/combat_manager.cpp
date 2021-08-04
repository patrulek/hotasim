#include "combat_manager.h"

#include "combat_ai.h"
#include "combat_state.h"
#include "combat_unit.h"

void CombatManager::moveUnit(CombatUnit& _unit, int _target_hex) {
	current_state->field.clearHex(_unit.getHex());
	// TODO: check double_wide
	_unit.moveTo(_target_hex);
	current_state->field.fillHex(_target_hex, CombatHexOccupation::UNIT);
	// TODO: check double_wide
}

void CombatManager::placeUnitsBeforeStart() {
	int unit_order = 0;
	for (auto& unit : current_state->attacker.getUnits()) {
		int hex = ai->pathfinder->getUnitStartHex(CombatSide::ATTACKER, unit_order++, current_state->attacker.getUnits().size(), unit->isDoubleWide(), combat_type);
		moveUnit(const_cast<CombatUnit&>(*unit), hex);
	}

	unit_order = 0;
	for (auto& unit : current_state->defender.getUnits()) {
		int hex = ai->pathfinder->getUnitStartHex(CombatSide::DEFENDER, unit_order++, current_state->defender.getUnits().size(), unit->isDoubleWide(), combat_type);
		moveUnit(const_cast<CombatUnit&>(*unit), hex);
	}
}

void CombatManager::createInitState() {
	CombatHero attacker_(*attacker);
	CombatHero defender_(*defender);
	CombatField field_(*field);

	init_state = std::make_unique<CombatState>(attacker_, defender_, field_);
	current_state = std::make_unique<CombatState>(attacker_, defender_, field_);
}

void CombatManager::setCurrentState(CombatState& _state) {
	current_state = std::make_unique<CombatState>(_state);
}

void CombatManager::initialize() {
	if (initialized)
		return;

	createInitState();
	placeUnitsBeforeStart();
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

const CombatHero& CombatManager::getAttacker() const { 
	return *attacker; 
}

const CombatHero& CombatManager::getDefender() const { 
	return *defender; 
}

CombatUnit& CombatManager::getActiveStack() {
	if (current_state->order.empty())
		throw std::exception("Cant get active stack from empty queue");

	int unit_id = current_state->order.front();
	auto& hero = unit_id / 21 == 0 ? current_state->attacker : current_state->defender;
	auto unit = const_cast<CombatUnit*>(hero.getUnits()[0]);
	return *unit;
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

void CombatManager::nextState() {
	if (!initialized)
		throw std::exception("Combat manager still didnt initialized");

	if (isNewCombat()) {
		nextStateByAction(createPreBattleAction());
		return;
	}

	if (isNewTurn()) {
		nextStateByAction(createPreTurnAction());
		return;
	}

	auto actions = generateActionsForAI();
	if (actions.empty())
		throw std::exception("Empty AI action list should never happen");
	
	nextStateByAction(actions.front()); // there is possibility for many actions which will create even more branching; TODO: later
}

#include <iostream>

void CombatManager::nextStateByAction(const CombatAction& action) {
	if (!initialized)
		throw std::exception("Combat manager still didnt initialized");

	last_action = std::make_unique<CombatAction>(action);
	last_state = std::make_unique<CombatState>(*current_state);

	bool next_unit = false;

	if (action.action == CombatActionType::PRE_BATTLE) {
		// TODO:
		// apply secondary skills for units from hero
		// apply precombat artifacts spells
		current_state->turn = 0;
		orderUnitsInTurn();
		std::cout << "Processed action: PRE_BATTLE\n";
	}
	else if (action.action == CombatActionType::PRE_TURN) {
		// reactivate spellbook
		// decrease spell active on units
		// place units in order
		++current_state->turn;
		orderUnitsInTurn();
		std::cout << "Processed action: PRE_TURN (" << current_state->turn << ")\n";
	}
	else if (action.action == CombatActionType::DEFENSE) {
		// todo:
		next_unit = action.param2;

		auto& active_stack = getActiveStack();
		std::cout << "Processed action: DEFENSE (" << active_stack.to_string().c_str() << ")\n";
	}

	setCombatResult();

	if( next_unit)
		nextUnit();
}

void CombatManager::nextUnit() {
	current_state->order.pop_front();
}

void CombatManager::setCombatResult() {
	bool player_alive = current_state->attacker.isAlive();
	bool enemy_alive = current_state->defender.isAlive();

	if (player_alive && enemy_alive) current_state->result = CombatResult::IN_PROGRESS;
	else if (player_alive)				current_state->result = CombatResult::PLAYER;
	else if (enemy_alive)				current_state->result = CombatResult::ENEMY;
	else										current_state->result = CombatResult::DRAW;
}

const bool CombatManager::isCombatFinished() const {
	return current_state->result != CombatResult::IN_PROGRESS && current_state->result != CombatResult::NOT_STARTED;
}

void CombatManager::orderUnitsInTurn()
{
	auto units = current_state->attacker.getUnits();
	auto defender_units = current_state->defender.getUnits();
	units.insert(std::end(units), std::begin(defender_units), std::end(defender_units));
	std::sort(std::begin(units), std::end(units), [](auto _obj1, auto _obj2) { return _obj1->currentStats.spd > _obj2->currentStats.spd; });

	current_state->order.clear();

	for (auto unit : units) {
		int offset = unit->getCombatSide() == CombatSide::ATTACKER ? 0 : 21;
		current_state->order.push_back(unit->getUnitId() + offset);
	}
}

bool CombatManager::isNewTurn()
{
	return current_state->order.empty();
}

bool CombatManager::isNewCombat()
{
	return current_state->result == CombatResult::NOT_STARTED;
}


CombatAction CombatManager::createPreBattleAction() const {
	return CombatAction{ CombatActionType::PRE_BATTLE, -1, -1, false };
}

CombatAction CombatManager::createPreTurnAction() const {
	return CombatAction{ CombatActionType::PRE_TURN, -1, -1, false };
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

std::vector<CombatAction> CombatManager::generateActionsForPlayer() {
	auto activeStack = getActiveStack();

	if (!activeStack.canMakeAction())
		return std::vector<CombatAction>{};

	std::vector<CombatAction> actions{};

	if (activeStack.canDefend())
		actions.push_back(createDefendAction());

	if (activeStack.canWait())
		actions.push_back(createWaitAction());

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

	// TODO: defense for now
	auto activeStack = getActiveStack();

	if (!activeStack.canMakeAction())
		return std::vector<CombatAction>{};

	std::vector<CombatAction> actions{};

	if (activeStack.canDefend())
		actions.push_back(createDefendAction());

	return actions;
}