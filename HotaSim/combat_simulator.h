#pragma once

#include <memory>
#include <cstdint>

#include "../HotaMechanics/structures.h"
#include "combat_permutation.h"

#include <list>

// for initialization
struct Hero;
enum class CombatFieldType;
enum class CombatType;

// for combat simulation
class CombatManager;
class CombatHero;
class CombatField;
class CombatState;
class CombatAction;

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

	int64_t evaluateCombatStateScore(const CombatState& _initial_state, const CombatState& _state) const;

	void setCombatManager(const CombatManager& _mgr);
	
	void updateBestState(const CombatState& _state, const std::list<CombatState>& _states, const std::list<CombatAction>& _actions);
private:
	void findBestAttackerPermutations();
	void setDefenderPermutation();

	void prepareCombat(const ArmyPermutation& _permutation, const int _field_template);
	std::shared_ptr<CombatField> prepareCombatField(const int _field_template);
	std::shared_ptr<CombatHero> prepareCombatHero(const Hero& _hero_template, const ArmyPermutation& _permutation, const CombatSide _side);

	// combat state score
	int64_t evaluateCombatStateAttackScore(const CombatState& _initial_state, const CombatState& _state) const;
	int64_t evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const;
	int64_t evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const;
	int64_t evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const;

	// during combat
	std::list<CombatState> states_timeline;
	std::list<CombatAction> actions_timeline;
	std::unique_ptr<CombatState> best_state;

	// before combat start
	std::unique_ptr<CombatManager> manager;

	// before simulation start
	std::vector<ArmyPermutation> permutations;
	ArmyPermutation defender_permutation;

	// for initialization
	std::unique_ptr<Hero> attacker;
	std::unique_ptr<Hero> defender;
	const CombatFieldType field_type;
	const CombatType combat_type;
};

