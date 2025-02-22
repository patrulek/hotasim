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
		explicit CombatManager(CombatHero&& attacker, CombatHero&& defender, CombatField&& field, const Constants::CombatType _combat_type);
		CombatManager(CombatManager&& _obj) = delete;
		CombatManager(const CombatManager& _obj) = delete;
		CombatManager() = delete;

		CombatManager& operator=(const CombatManager& _obj) = delete;
		~CombatManager();

		// state machine ----------------
		void initialize();
		void reinitialize();
		void nextState();
		void nextStateByAction(const CombatAction& _action);
		void setCurrentState(const CombatStatePacked& _current_state);
		const bool isUnitMove() const;
		const bool isPlayerMove();
		const bool isCombatFinished() const;
		const bool didPlayerWon() const;
		// ------------------------------

		// action generator -------------
		const std::vector<CombatAction> generateActionsForPlayer();
		const std::vector<CombatAction> generateActionsForAI();
		// ------------------------------

		// unit utils -------------------
		CombatUnit& getActiveStack();
		CombatUnit& getStackByGlobalId(const int _guid) const;
		CombatUnit& getStackByLocalId(const int _uid, const Constants::CombatSide _side) const;
		const std::vector<const CombatUnit*>& getAllUnitStacks() const;
		void setAllUnitStacks();
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
		const CombatAction createWalkAction(uint8_t _hex_id, int16_t _walk_distance = -1) const;
		const CombatAction createDefendAction() const;
		const CombatAction createSpellCastAction(int16_t _spell_id, int16_t _unit_id, uint8_t _hex_id) const;
		const CombatAction createAttackAction(int16_t _unit_id, uint8_t _hex_id) const;
		// ------------------------------

		// event creator ----------------
		const CombatEvent createUnitStatsChangedEvent(const int16_t _unit_id, const uint8_t _stats_id) const;
		const CombatEvent createUnitHealthLostEvent(const int16_t _unit_id) const;
		const CombatEvent createUnitPosChangedEvent(const int16_t _unit_id, const uint8_t _source_hex, const uint8_t _target_hex) const;
		const CombatEvent createFieldChangeEvent(const std::vector<uint8_t>& _hexes, const bool _destroy = true) const;
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
		void reorderUnits();
		void removeFromOrderList(const int _unit_id);
		void restoreLastUnit();
		void nextUnit();
		// -----------------------------

		// unit utils ------------------
		void makeUnitAttack(int _unit_id, uint8_t _target_hex);
		void makeUnitFly(uint8_t _target_hex);
		void makeUnitWalk(uint8_t _target_hex, int _walk_distance = -1);
		void makeUnitDefend();
		void makeUnitWait();
		const std::vector<CombatUnit*> getUnitsInRange(const Constants::CombatSide _side, const std::vector<uint8_t>& _hexes) const;
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

		// during battle
		std::vector<CombatEvent> action_events;
		std::vector<const CombatUnit*> all_units;
		const CombatUnit* active_stack{ nullptr };
		std::vector<CombatAction> actions;
		std::vector<uint8_t> hexes_to_attack;
		

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
