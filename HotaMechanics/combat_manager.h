#pragma once

#include <memory>

#include "structures.h"

#include "combat_hero.h"
#include "combat_field.h"
#include "combat_state.h"
#include "combat_ai.h"

namespace HotaMechanics {

	class CombatManager {

	public:
		explicit CombatManager(const CombatHero& attacker, const CombatHero& defender, const CombatField& field, const Constants::CombatType _combat_type);
		CombatManager(CombatManager&& _obj) = delete;
		CombatManager(const CombatManager& _obj) = delete;
		CombatManager() = delete;

		CombatManager& operator=(const CombatManager& _obj) = delete;
		~CombatManager();

		// state machine ----------------
		void initialize();
		void nextState();
		void nextStateByAction(const CombatAction& _action);
		void setCurrentState(const CombatState& _current_state);
		const bool isUnitMove() const;
		const bool isPlayerMove() const;
		const bool isCombatFinished() const;
		// ------------------------------

		// action generator -------------
		const std::vector<CombatAction> generateActionsForPlayer();
		const std::vector<CombatAction> generateActionsForAI();
		// ------------------------------

		// simple getters ---------------
		const CombatHero& getAttacker() const { return *attacker; }
		const CombatHero& getDefender() const { return *defender; }
		const Constants::CombatType getCombatType() const { return combat_type; }

		CombatState& getCurrentState() const { return *current_state; }
		const CombatState& getInitialState() const { return *init_state; }
		const CombatAI& getCombatAI() const { return *ai; }
		const CombatField& getInitialCombatField() const { return *field; }
		// ------------------------------
	private:
		// action creator ---------------
		const CombatAction createPreBattleAction() const;
		const CombatAction createPreTurnAction() const;
		const CombatAction createWaitAction() const;
		const CombatAction createWalkAction(int16_t _hex_id, int16_t _walk_distance = -1) const;
		const CombatAction createDefendAction() const;
		const CombatAction createSpellCastAction(int16_t _spell_id, int16_t _unit_id, int16_t _hex_id) const;
		const CombatAction createAttackAction(int16_t _unit_id, int16_t _hex_id) const;
		// ------------------------------

		// action processor -------------
		void processPreBattleAction(const CombatAction& _action);
		void processPreTurnAction(const CombatAction& _action);
		void processWaitAction(const CombatAction& _action);
		void processWalkAction(const CombatAction& _action);
		void processDefendAction(const CombatAction& _action);
		void processSpellCastAction(const CombatAction& _action);
		void processAttackAction(const CombatAction& _action);

		void processCombatAction(const CombatAction& _action);
		void processUnitAction(const CombatAction& _action);

		void orderUnitsInTurn();
		void removeFromOrderList(const int _unit_id);
		void restoreLastUnit();
		void nextUnit();
		// -----------------------------

		// unit utils ------------------
		CombatUnit& getActiveStack() const;
		void moveUnit(CombatUnit& _unit, int _target_hex);
		void makeUnitAttack(int _unit_id, int _target_hex);
		void makeUnitFly(int _target_hex);
		void makeUnitWalk(int _target_hex, int _walk_distance = -1);
		void makeUnitDefend();
		void makeUnitWait();
		const std::vector<CombatUnit*> getUnitsInRange(const Constants::CombatSide _side, const std::vector<int>& _hexes) const;
		// -----------------------------

		// state utils -----------------
		void createInitState();
		const bool isNewTurn() const;
		const bool isNewCombat() const;
		const bool isUnitAction(const CombatAction& _action);
		void placeUnitsBeforeStart();
		void setCombatResult();
		const bool isInitialTacticsState() const;
		// -----------------------------

		// post-initialization
		bool initialized{ false };
		bool state_changed{ false };
		std::unique_ptr<CombatState> init_state;
		std::unique_ptr<CombatState> current_state;

		// pre-initialization
		std::unique_ptr<CombatHero> attacker;
		std::unique_ptr<CombatHero> defender;
		std::unique_ptr<CombatField> field;
		const Constants::CombatType combat_type{ Constants::CombatType::NEUTRAL };
		std::unique_ptr<CombatAI> ai;
	};
}; // HotaMechanics
