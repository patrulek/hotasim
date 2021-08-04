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
	explicit CombatManager(const CombatHero& attacker, const CombatHero& defender, const CombatField& field, const CombatType _combat_type);
	CombatManager(CombatManager&& _obj) = delete;
	CombatManager(const CombatManager& _obj) = delete;
	CombatManager() = delete;

	CombatManager& operator=(const CombatManager& _obj) = delete;

	~CombatManager();




	void initialize();


	CombatUnit& getActiveStack();

	const bool isCombatFinished() const;

	const CombatHero& getAttacker() const;
	const CombatHero& getDefender() const;
	const CombatType getCombatType() const { return combat_type; }

	CombatState& getCurrentState() const { return *current_state; }
	const CombatState& getInitialState() const { return *init_state; }
	const CombatAction& getLastAction() const { return *last_action; }
	const CombatAI& getCombatAI() const;
	const CombatField& getCombatField() const;

	std::vector<CombatUnit> getUnitsInRange(CombatSide side, std::vector<int>& hexes) const;

	void setCurrentState(CombatState& _current_state);

	//CombatUnit& nextUnit(CombatState& state) {
		// get next unit in turn
		// 

	//	return const_cast<CombatUnit&>(*state.heroes[0].getUnits()[0]); // 
	//}


	bool isPlayerMove() {
		try {
			return getActiveStack().getCombatSide() == CombatSide::ATTACKER;
		}
		catch(const std::exception&) {
			return false;
		}
	}

	bool isTacticsState(CombatState& state) const {
		return false;
	}

	// 1EC62
	//int getSideStacksNumber(CombatState& state, int side) const {
	//	return state.heroes[side].aliveStacks(state.heroes[side]);
	//}

	void nextUnit();

	void nextState();
	void nextStateByAction(const CombatAction& action);

	// action utils
	std::vector<CombatAction> generateActionsForPlayer();
	std::vector<CombatAction> generateActionsForAI();
private:
	void setCombatResult();

	void orderUnitsInTurn();


	bool isNewTurn();

	bool isNewCombat();

	CombatAction createPreTurnAction() const;
	CombatAction createPreBattleAction() const;
	CombatAction createWaitAction() const;
	CombatAction createWalkAction(int hex_id) const;
	CombatAction createDefendAction() const;
	CombatAction createSpellCastAction(int spell_id, int unit_id, int hex_id) const;
	CombatAction createAttackAction(int unit_id, int hex_id) const;


	// state utils
	void createInitState();

	// unit utils
	void placeUnitsBeforeStart();
	void moveUnit(CombatUnit& _unit, int _target_hex);

	// during combat

	// post-initialization
	bool initialized{ false };
	bool state_changed{ false };
	std::unique_ptr<CombatAction> last_action;
	std::unique_ptr<CombatState> last_state;
	std::unique_ptr<CombatState> init_state;
	std::unique_ptr<CombatState> current_state;

	// pre-initialization
	std::unique_ptr<CombatHero> attacker;
	std::unique_ptr<CombatHero> defender;
	std::unique_ptr<CombatField> field;
	CombatType combat_type;
	std::unique_ptr<CombatAI> ai;
};
