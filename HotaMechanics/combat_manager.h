#pragma once

#include <memory>

#include "structures.h"

#include "combat_state.h"
#include "combat_action.h"

class CombatAI;
class CombatHero;
class CombatField;
class CombatUnit;

class CombatManager {

public:
	CombatManager(const CombatHero& attacker, const CombatHero& defender, const CombatField& field, const CombatType _combat_type);
	~CombatManager();

	void initialize();

	CombatState nextState();

	CombatUnit& getActiveStack();


	CombatState duplicateCurrentState() const { return CombatState(*current_state); }
	CombatState& getCurrentState() const { return *current_state; }
	//const CombatState& getInitialState() const { return init_state; }
	const CombatAI& getCombatAI() const;
	const CombatField& getCombatField() const;

	std::vector<CombatUnit> getUnitsInRange(CombatSide side, std::vector<int>& hexes) const;

	int calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const;
	int calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const;
	int calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender) const;
	std::vector<int> calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero) const;

	void setCurrentState(CombatState& _current_state) { current_state = &_current_state; }

	//CombatUnit& nextUnit(CombatState& state) {
		// get next unit in turn
		// 

	//	return const_cast<CombatUnit&>(*state.heroes[0].getUnits()[0]); // 
	//}

	bool isUnitMove(CombatState& state) {
		return state.currentUnit != -1; // check if unit move or turnupdate
	}

	bool isPlayerMove(CombatState& state) {
		return isUnitMove(state) && state.unitOrder[state.currentUnit] < 21;
	}

	void resolveAction(CombatState& state, CombatAction action) {

	}

	bool isTacticsState(CombatState& state) const {
		return false;
	}

	// 1EC62
	//int getSideStacksNumber(CombatState& state, int side) const {
	//	return state.heroes[side].aliveStacks(state.heroes[side]);
	//}

	CombatState nextStateByAction(CombatState& state, CombatAction action) {
		//CombatState state_ = duplicate(state);
		//resolveAction(state_, action);
		//return state_;
	}

private:
	void setCombatResult();

	void orderUnitsInTurn();
	void preTurnUpdate();
	void updateTurn();

	void updateCombat();


	bool isNewTurn();

	bool isNewCombat();

	void preCombatUpdate() {
		// update combat field
		// place units over combat field
		// apply secondary skills for units from hero
		// apply precombat artifacts spells
	}

	// state utils
	void createInitState();

	// unit utils
	void placeUnitsBeforeStart();
	void moveUnit(CombatUnit& _unit, int _target_hex);

	// post-initialization
	bool initialized{ false };
	std::unique_ptr<CombatState> init_state;
	CombatState* current_state;

	// pre-initialization
	std::unique_ptr<CombatHero> attacker;
	std::unique_ptr<CombatHero> defender;
	std::unique_ptr<CombatField> field;
	const CombatType combat_type;
	std::unique_ptr<CombatAI> ai;
};

/*
FLOW:

CombatState initState = initCombat(...); // before battle

CombatState state = initState;

while( state.result == (NOT_STARTED || IN_PROGRESS) ) {
	if( isUnitMove(state) )
		state = nextStateByAction(nextState, action);
	else
		state = nextState(state);
}


*/