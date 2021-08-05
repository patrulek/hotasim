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
	const CombatField& getInitialCombatField() const;

	std::vector<CombatUnit*> getUnitsInRange(CombatSide side, std::vector<int>& hexes) const;

	void setCurrentState(CombatState& _current_state);


	bool isUnitMove() {
		return !isNewCombat() && !isNewTurn();
	}

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

	void removeFromOrderList(const int _unit_id);
	void restoreLastUnit();
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

	const bool isUnitAction(const CombatAction& _action);
	

	CombatAction createPreBattleAction() const;
	CombatAction createPreTurnAction() const;
	CombatAction createWaitAction() const;
	CombatAction createWalkAction(int hex_id, int _walk_distance = -1) const;
	CombatAction createDefendAction() const;
	CombatAction createSpellCastAction(int spell_id, int unit_id, int hex_id) const;
	CombatAction createAttackAction(int unit_id, int hex_id) const;

	void processPreBattleAction(const CombatAction& _action);
	void processPreTurnAction(const CombatAction& _action);
	void processWaitAction(const CombatAction& _action);
	void processWalkAction(const CombatAction& _action);
	void processDefendAction(const CombatAction& _action);
	void processSpellCastAction(const CombatAction& _action);
	void processAttackAction(const CombatAction& _action);

	void processCombatAction(const CombatAction& _action);
	void processUnitAction(const CombatAction& _action);

	// state utils
	void createInitState();

	// unit utils
	void placeUnitsBeforeStart();
	void moveUnit(CombatUnit& _unit, int _target_hex);
	void makeUnitAttack(int _unit_id, int _target_hex);
	void makeUnitFly(int _target_hex);
	void makeUnitWalk(int _target_hex, int _walk_distance = -1);
	void makeUnitDefend();
	void makeUnitWait();

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
