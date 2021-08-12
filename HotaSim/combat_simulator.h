#pragma once

#include <memory>
#include <cstdint>

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/constants.h"

#include <list>


using namespace HotaMechanics;
using namespace HotaMechanics::Constants;

// for combat simulation
namespace HotaMechanics {
	class CombatManager;
	class CombatHero;
	class CombatField;
	struct CombatState;
	struct CombatAction;
}

namespace HotaSim {
	class CombatSequenceTree;

	class CombatSimulator
	{
	public:
		explicit CombatSimulator(const Hero& _attacker, const Hero& _defender, const CombatFieldType _field_type, const CombatType _combat_type);
		CombatSimulator(const CombatSimulator& _obj) = delete;
		CombatSimulator(CombatSimulator&& _obj) = delete;
		CombatSimulator() = delete;

		CombatSimulator& operator=(const CombatSimulator& _obj) = delete;
		CombatSimulator& operator=(CombatSimulator&& _obj) = delete;

		~CombatSimulator();

		void initialize();
		void start();

		uint64_t evaluateCombatStateScore(const CombatState& _initial_state, const CombatState& _state) const;

		void setCombatManager(const CombatManager& _mgr);

		void updateBestState(const CombatState& _state, const std::list<CombatState>& _states, const std::list<CombatAction>& _actions);
	private:
		void findBestAttackerPermutations();
		void setDefenderPermutation();

		const bool simulatorConstraintsViolated(const CombatSequenceTree& _tree);
		const bool combatConstraintsViolated(const CombatSequenceTree& _tree);
		void resetRulesCounters();

		void prepareCombat(const ArmyPermutation& _permutation, const CombatFieldTemplate _field_template = CombatFieldTemplate::IMPS_2x100);
		std::shared_ptr<CombatField> prepareCombatField(const CombatFieldTemplate _field_template);
		std::shared_ptr<CombatHero> prepareCombatHero(const Hero& _hero_template, const ArmyPermutation& _permutation, const CombatSide _side);

		// combat state score
		uint64_t evaluateCombatStateAttackScore(const CombatState& _initial_state, const CombatState& _state) const;
		uint64_t evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const;
		uint64_t evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const;
		uint64_t evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const;

		// during combat
		std::unique_ptr<CombatState> best_state;

		// before combat start
		std::unique_ptr<CombatManager> manager;
		int combat_finished_cnt;
		int estimated_turns;
		int turns_rule_violation_cnt;
		int estimated_total_states;

		// before simulation start
		std::vector<ArmyPermutation> permutations;
		ArmyPermutation defender_permutation;

		// for initialization
		std::unique_ptr<Hero> attacker;
		std::unique_ptr<Hero> defender;
		const CombatFieldType field_type;
		const CombatType combat_type;
	};

}; // HotaSim