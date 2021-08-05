#include "combat_manager.h"

#include "combat_ai.h"
#include "combat_state.h"
#include "combat_unit.h"

#include <iostream>
#include <algorithm>

void CombatManager::makeUnitDefend() {
	auto& active_stack = getActiveStack();
	active_stack.defend();
}

void CombatManager::makeUnitWait() {
	auto& active_stack = getActiveStack();
	active_stack.wait();
	current_state->order.push_back(current_state->order.front());

	if (current_state->order.size() > 2) {
		for (auto it = std::rbegin(current_state->order), it2 = it; ; ++it) {
			std::advance(it2, 1);
			int cur = *it, prev = *(it2);
			auto& hero = prev / 21 == 0 ? current_state->attacker : current_state->defender;
			auto& unit = hero.getUnits()[prev % 21];

			if (!unit->state.waiting)
				break;

			if (!unit->currentStats.spd > active_stack.currentStats.spd) {
				std::swap(*it, *it2);
			}
		}
	}
}

void CombatManager::makeUnitFly(int _target_hex) {
	throw std::exception("Not implemented yet");
}

void CombatManager::makeUnitWalk(int _target_hex, int _walk_distance) {
	auto& active_stack = getActiveStack();
	auto path = ai->pathfinder->findPath(active_stack.getHex(), _target_hex, current_state->field);

	if (path.empty())
		throw std::exception("Should never happen (we already found that path earlier)");

	int walk_distance = _walk_distance == -1 ? active_stack.currentStats.spd : _walk_distance;
	int range = std::min(path.size(), (size_t)walk_distance);
	for (int i = 0; i < range; ++i) {
		std::cout << "Moved unit from (" << active_stack.getHex() << ") to (" << path[i] << ")\n";
		moveUnit(active_stack, path[i]);
	}

	active_stack.state.done = true; // TODO: shouldnt be done yet if we're attacking, unless quicksand or smth
}

void CombatManager::makeUnitAttack(int _unit_id, int _target_hex) {
	auto& active_stack = getActiveStack();
	if (active_stack.getHex() != _target_hex) {
		makeUnitWalk(_target_hex);
		active_stack.state.done = false; // TODO: need to fix it somehow
	}

	if (!active_stack.isAlive())
		return;

	auto defender = active_stack.getCombatSide() == CombatSide::ATTACKER ? current_state->defender.getUnits()[_unit_id] : current_state->attacker.getUnits()[_unit_id];
	int dmg = ai->calculateMeleeUnitAverageDamage(active_stack, *defender);
	int counter_dmg = ai->calculateCounterAttackMeleeUnitAverageDamage(active_stack, *defender);


	std::cout << "(" << active_stack.to_string() << ") attack (" << defender->to_string() << ")\n";
	const_cast<CombatUnit*>(defender)->applyDamage(dmg);
	std::cout << "(" << active_stack.to_string() << ") dealt " << dmg << " to (" << defender->to_string() << ")\n";

	if (defender->isAlive()) {
		if (!defender->state.retaliated) {
			std::cout << "(" << defender->to_string() << ") counter attack (" << active_stack.to_string() << ")\n";
			active_stack.applyDamage(counter_dmg);
			std::cout << "(" << defender->to_string() << ") counter attack dealt " << counter_dmg << " to (" << active_stack.to_string() << ")\n";
			const_cast<CombatUnit*>(defender)->state.retaliated = true;
		}
	}
	else {
		removeFromOrderList(_unit_id + (defender->getCombatSide() == CombatSide::ATTACKER ? 0 : 21));
	}

	if( active_stack.isAlive())
		active_stack.state.done = true;
}

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

std::vector<CombatUnit*> CombatManager::getUnitsInRange(CombatSide side, std::vector<int>& hexes) const {
	std::vector<CombatUnit*> units_in_range;
	auto& hero = side == CombatSide::ATTACKER ? current_state->attacker : current_state->defender;

	for (auto& unit : hero.getUnits()) {
		if (!unit->isAlive())
			continue;

		// if unit isnt common unit -> continue

		bool found = std::find(std::begin(hexes), std::end(hexes), unit->getHex()) != std::end(hexes);
		if (found)
			units_in_range.push_back(const_cast<CombatUnit*>(unit));
	}

	return units_in_range;
}

const CombatField& CombatManager::getInitialCombatField() const {
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

	auto unit = const_cast<CombatUnit*>(hero.getUnits()[unit_id % 21]);
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

	throw std::exception("Should never happen (unit move, but no action provided)");
}


void CombatManager::nextStateByAction(const CombatAction& action) {
	if (!initialized)
		throw std::exception("Combat manager still didnt initialized");

	last_action = std::make_unique<CombatAction>(action);
	last_state = std::make_unique<CombatState>(*current_state);

	if (isUnitAction(action))
		processUnitAction(action);
	else
		processCombatAction(action);

	setCombatResult();

	if( action.param2)
		nextUnit();
}

void CombatManager::removeFromOrderList(const int _unit_id) {
	current_state->order.remove(_unit_id);
}

const bool CombatManager::isUnitAction(const CombatAction& _action) {
	return _action.action != CombatActionType::PRE_BATTLE && _action.action != CombatActionType::PRE_TURN;
}

void CombatManager::processUnitAction(const CombatAction& _action) {
	if (_action.action == CombatActionType::DEFENSE)
		processDefendAction(_action);
	else if (_action.action == CombatActionType::WAIT)
		processWaitAction(_action);
	else if (_action.action == CombatActionType::WALK)
		processWalkAction(_action);
	else if (_action.action == CombatActionType::SPELLCAST)
		processSpellCastAction(_action);
	else if (_action.action == CombatActionType::ATTACK)
		processAttackAction(_action);
}

void CombatManager::processCombatAction(const CombatAction& _action) {
	if (_action.action == CombatActionType::PRE_BATTLE)
		processPreBattleAction(_action);
	else if (_action.action == CombatActionType::PRE_TURN)
		processPreTurnAction(_action);
}

void CombatManager::processPreBattleAction(const CombatAction& _action) {
	// TODO:
		// apply secondary skills for units from hero
		// apply precombat artifacts spells
	current_state->turn = 0;
	orderUnitsInTurn();
	std::cout << "Processed action: PRE_BATTLE\n\n-------------------\n\n";
}

void CombatManager::processPreTurnAction(const CombatAction& _action) {
	for (auto& unit : current_state->attacker.getUnits())
		const_cast<CombatUnit*>(unit)->resetState();

	for (auto& unit : current_state->defender.getUnits())
		const_cast<CombatUnit*>(unit)->resetState();

	// reactivate spellbook
		// decrease spell active on units
	++current_state->turn;
	orderUnitsInTurn();
	std::cout << "Processed action: PRE_TURN (" << current_state->turn << ")\n\n-----------------\n\n";
}

void CombatManager::processWaitAction(const CombatAction& _action) {
	makeUnitWait();

	auto& active_stack = getActiveStack();
	std::cout << "Processed action: WAIT (" << active_stack.to_string().c_str() << ")\n\n";
}

void CombatManager::processWalkAction(const CombatAction& _action) {
	auto& active_stack = getActiveStack();
	if (active_stack.canFly())
		makeUnitFly(_action.target);
	else
		makeUnitWalk(_action.target, _action.param1);
	std::cout << "Processed action: WALK (" << active_stack.to_string().c_str() << ") to pos: " << _action.target << "\n\n";
}

void CombatManager::processDefendAction(const CombatAction& _action) {
	makeUnitDefend();

	auto& active_stack = getActiveStack();
	std::cout << "Processed action: DEFENSE (" << active_stack.to_string().c_str() << ")\n\n";
}

void CombatManager::processSpellCastAction(const CombatAction& _action) {
	std::cout << "Processed action: SPELLCAST (need to be implemented)\n";
}

void CombatManager::processAttackAction(const CombatAction& _action) {
	makeUnitAttack(_action.param1, _action.target);

	auto& active_stack = getActiveStack();
	std::cout << "Processed action: ATTACK (" << active_stack.to_string().c_str() << ") on unit: " 
				 << _action.param1 << " at pos: " << _action.target << "\n\n";
}

void CombatManager::restoreLastUnit() {
	current_state->order.push_front(current_state->last_unit);
	current_state->last_unit = -1;
}

void CombatManager::nextUnit() {
	current_state->last_unit = current_state->order.front();
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
		if (!unit->isAlive())
			continue;

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

CombatAction CombatManager::createWalkAction(int hex_id, int _walk_distance) const {
	return CombatAction{ CombatActionType::WALK, _walk_distance, hex_id, true };
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
	auto& field = current_state->field;
	auto range_hexes = ai->pathfinder->getHexesInRange(activeStack.getHex(), activeStack.currentStats.spd);
	auto walkable_hexes = ai->pathfinder->getWalkableHexesFromList(range_hexes, field);
	auto reachable_hexes = ai->pathfinder->getReachableHexesFromWalkableHexes(activeStack.getHex(), activeStack.currentStats.spd, walkable_hexes, false, false, field);

	for (auto hex : reachable_hexes)
		actions.push_back(createWalkAction(hex));

	// get attackable enemy units; 
	// if can shoot then only get all enemy units
	auto units_in_range = getUnitsInRange(CombatSide::DEFENDER, range_hexes);

	for (auto unit : units_in_range) {
		auto adjacent_hexes = ai->pathfinder->getAdjacentHexesClockwise(unit->getHex());
		auto adjacent_vec = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		auto walkable_adjacent = ai->pathfinder->getWalkableHexesFromList(adjacent_vec, field);
		auto reachable_adjacent = ai->pathfinder->getReachableHexesFromWalkableHexes(activeStack.getHex(), activeStack.currentStats.spd,
			walkable_adjacent, false, false, field);


		// if were staying in one of adjacent hexes to target, add that hex
		bool staying_around = std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), activeStack.getHex()) != std::end(adjacent_hexes);
		if (staying_around)
			reachable_adjacent.push_back(activeStack.getHex());

		for (auto hex : reachable_adjacent) {
			actions.push_back(createAttackAction(unit->getUnitId(), hex));
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
	auto active_stack = getActiveStack();

	if (!active_stack.canMakeAction())
		return std::vector<CombatAction>{};

	std::vector<CombatAction> actions{};
	ai->calculateFightValueAdvantageOnHexes(active_stack, current_state->attacker, current_state->field);
	auto units_to_attack = ai->chooseUnitToAttack(active_stack, current_state->attacker);

	for (auto unit_id : units_to_attack) {
		auto unit = current_state->attacker.getUnits()[unit_id];
		int hex = ai->chooseHexToMoveForAttack(active_stack, *unit);

		int distance_to_hex = ai->pathfinder->findPath(active_stack.getHex(), hex, current_state->field).size();
		int turns = distance_to_hex == 0 ? 1 : std::ceil((float)distance_to_hex / active_stack.currentStats.spd);

		if (turns == 1)
			actions.push_back(createAttackAction(unit_id, hex));
		else {
			int walk_distance = ai->chooseWalkDistanceFromPath(active_stack, hex, current_state->field);

			if (walk_distance == 0) {
				if (active_stack.canWait())
					actions.push_back(createWaitAction());
				else
					actions.push_back(createWalkAction(hex));
			}
			else {
				actions.push_back(createWalkAction(hex, walk_distance));
			}
		}
	}



	/*
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
	auto units_in_range = getUnitsInRange(CombatSide::ATTACKER, range_hexes);

	for (auto unit : units_in_range) {
		auto adjacent_hexes = ai->pathfinder->getAdjacentHexesClockwise(unit->getHex());
		auto adjacent_vec = std::vector<int>(std::begin(adjacent_hexes), std::end(adjacent_hexes));
		auto walkable_adjacent = ai->pathfinder->getWalkableHexesFromList(adjacent_vec, field);
		auto reachable_adjacent = ai->pathfinder->getReachableHexesFromWalkableHexes(activeStack.getHex(), activeStack.currentStats.spd,
			walkable_adjacent, false, false, field);


		// if were staying in one of adjacent hexes to target, add that hex
		bool staying_around = std::find(std::begin(adjacent_hexes), std::end(adjacent_hexes), activeStack.getHex()) != std::end(adjacent_hexes);
		if (staying_around)
			reachable_adjacent.push_back(activeStack.getHex());

		for (auto hex : reachable_adjacent) {
			actions.push_back(createAttackAction(unit->getUnitId(), hex));
		}
	}

	// get castable spells;
	if (activeStack.canCast())
		throw std::exception("Not implemented yet");

	if (activeStack.canHeroCast())
		throw std::exception("Not implemented yet");
	*/
	return actions;
}