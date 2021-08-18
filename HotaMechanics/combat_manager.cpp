#include "combat_manager.h"

#include <iostream>
#include <algorithm>

#include "combat_calculator.h"
#include "combat_unit.h"
#include "combat_ai.h"

namespace HotaMechanics {
	using namespace Calculator;
	using namespace Constants;

	CombatManager::CombatManager(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field, const CombatType _combat_type)
		: combat_type(_combat_type)
	{
		attacker = std::make_unique<CombatHero>(_attacker);
		defender = std::make_unique<CombatHero>(_defender);
		field = std::make_unique<CombatField>(_field);
		ai = std::make_unique<CombatAI>(*this);

		actions.reserve(128);
		action_events.reserve(32);
		hexes_to_attack.reserve(32);
		all_units.reserve(32);
	}

	CombatManager::CombatManager(CombatHero&& _attacker, CombatHero&& _defender, CombatField&& _field, const CombatType _combat_type)
		: combat_type(_combat_type)
	{
		attacker = std::make_unique<CombatHero>(std::move(_attacker));
		defender = std::make_unique<CombatHero>(std::move(_defender));
		field = std::make_unique<CombatField>(std::move(_field));
		ai = std::make_unique<CombatAI>(*this);

		actions.reserve(128);
		hexes_to_attack.reserve(32);
		action_events.reserve(32);
		all_units.reserve(32);
	}

	CombatManager::~CombatManager() {}

	void CombatManager::reinitialize() {
		initialized = false;
		initialize();
	}

	void CombatManager::initialize() {
		if (initialized)
			return;

		createInitState();
		placeUnitsBeforeStart();
		//const_cast<CombatField&>(current_state->field).rehash();
		initialized = true;
	}

	void CombatManager::createInitState() {
		CombatHero attacker_(*attacker);
		CombatHero defender_(*defender);
		CombatField field_(*field);

		init_state = std::make_unique<CombatState>(attacker_, defender_, field_);
		current_state = std::make_unique<CombatState>(attacker_, defender_, field_);
	}

	void CombatManager::placeUnitsBeforeStart() {
		int unit_order = 0;
		for (auto& unit : current_state->attacker.getUnitsPtrs()) {
			const auto hex = ai->getPathfinder().getUnitStartHex(CombatSide::ATTACKER, unit_order++, (int16_t)current_state->attacker.getUnitsPtrs().size(), unit->isDoubleWide(), combat_type);
			const_cast<CombatUnit*>(unit)->moveTo(hex);
			current_state->field.fillHex(hex, CombatHexOccupation::UNIT);
		}

		unit_order = 0;
		for (auto& unit : current_state->defender.getUnitsPtrs()) {
			const auto hex = ai->getPathfinder().getUnitStartHex(CombatSide::DEFENDER, unit_order++, (int16_t)current_state->defender.getUnitsPtrs().size(), unit->isDoubleWide(), combat_type);
			const_cast<CombatUnit*>(unit)->moveTo(hex);
			current_state->field.fillHex(hex, CombatHexOccupation::UNIT);
		}

		setAllUnitStacks();
	}

	void CombatManager::setAllUnitStacks() {
		all_units.clear();
		auto& attacker_units = current_state->attacker.getUnitsPtrs();
		auto& defender_units = current_state->defender.getUnitsPtrs();

		std::copy(std::begin(attacker_units), std::end(attacker_units), std::back_inserter(all_units));
		std::copy(std::begin(defender_units), std::end(defender_units), std::back_inserter(all_units));
	}

	void CombatManager::setCurrentState(const CombatStatePacked& _state) {
		unpackCombatState(_state);
		setAllUnitStacks();
		//const_cast<CombatField&>(current_state->field).rehash();
		//ai->initializeBattle(nullptr, nullptr, nullptr, nullptr, true);
		//const_cast<CombatPathfinder&>(ai->getPathfinder()).clearCache();
	}

	void CombatManager::nextState() {
		if (!initialized)
			throw std::exception("Combat manager still didnt initialized");

		if (isNewCombat()) {
			nextStateByAction(createPreBattleAction());
			ai->initializeBattle();
			return;
		}

		if (isNewTurn()) {
			nextStateByAction(createPreTurnAction());
			return;
		}

		throw std::exception("Should never happen (unit move, but no action provided)");
	}


	const std::vector<const CombatUnit*>& CombatManager::getAllUnitStacks() const {
		if (!all_units.empty()) // TODO: change when new mechanics added (summon/clone)
			return all_units;

		throw std::exception("should never happen");
	}

	void CombatManager::nextStateByAction(const CombatAction& action) {
		if (!initialized)
			throw std::exception("Combat manager still didnt initialized");

		action_events.clear();

		if (isUnitAction(action))
			processUnitAction(action);
		else
			processCombatAction(action);

		ai->addEventsToProcess(action_events);
		ai->processEvents();

		setCombatResult();

		if (action.param2)
			nextUnit();
	}

	const std::vector<CombatUnit*> CombatManager::getUnitsInRange(const CombatSide _side, const std::vector<uint8_t>& _hexes) const {
		std::vector<CombatUnit*> units_in_range;
		auto& hero = _side == CombatSide::ATTACKER ? current_state->attacker : current_state->defender;

		for (auto& unit : hero.getUnitsPtrs()) {
			if (!unit->isAlive())
				continue;

			// if unit isnt common unit -> continue

			bool found = std::find(std::begin(_hexes), std::end(_hexes), unit->getHex()) != std::end(_hexes);
			if (found)
				units_in_range.push_back(const_cast<CombatUnit*>(unit));
		}

		return units_in_range;
	}

	const bool CombatManager::isUnitAction(const CombatAction& _action) {
		return _action.action != CombatActionType::PRE_BATTLE && _action.action != CombatActionType::PRE_TURN;
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

	const bool CombatManager::isNewTurn() const {
		return current_state->order.empty();
	}

	const bool CombatManager::isNewCombat() const  {
		return current_state->result == CombatResult::NOT_STARTED;
	}

	const bool CombatManager::isUnitMove() const {
		return !isNewCombat() && !isNewTurn();
	}

	const bool CombatManager::isPlayerMove() {
		try {
			return getActiveStack().getCombatSide() == CombatSide::ATTACKER;
		}
		catch (const std::exception&) {
			return false;
		}
	}
	
	const bool CombatManager::isInitialTacticsState() const {
		return false;
	}
}; // HotaMechanics