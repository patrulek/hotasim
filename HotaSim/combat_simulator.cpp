#include "combat_simulator.h"

#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_ai.h"
#include "../HotaMechanics/combat_state.h"
#include "../HotaMechanics/combat_action.h"

#include <list>
#include <iostream>

CombatSimulator::CombatSimulator(const Hero& _attacker, const Hero& _defender, 
											const CombatFieldType _field_type, const CombatType _combat_type)
	: field_type(_field_type), combat_type(_combat_type) {
	
	attacker = std::make_unique<Hero>(_attacker);
	defender = std::make_unique<Hero>(_defender);
}

CombatSimulator::~CombatSimulator() {}

void CombatSimulator::updateBestState(const CombatState& _state, const std::list<CombatState>& _states, const std::list<CombatAction>& _actions) {
	if (evaluateCombatStateScore(manager->getInitialState(), _state) <= evaluateCombatStateScore(manager->getInitialState(), *best_state))
		return;

	best_state = std::make_unique<CombatState>(_state);
	states_timeline = _states;
	actions_timeline = _actions;
}

void CombatSimulator::setCombatManager(const CombatManager& _mgr) { 
	manager = std::make_unique<CombatManager>(_mgr.getAttacker(), _mgr.getDefender(), _mgr.getInitialCombatField(), _mgr.getCombatType());
}

int64_t CombatSimulator::evaluateCombatStateScore(const CombatState& _initial_state, const CombatState& _state) const {
	int64_t attack_score = evaluateCombatStateAttackScore(_initial_state, _state); // the more enemy units we killed the better
	int64_t defense_score = evaluateCombatStateDefenseScore(_initial_state, _state); // the less units we lost the better
	int64_t turns_score = evaluateCombatStateTurnsScore(_initial_state, _state); // the less turns we fought the better
	int64_t mana_score = evaluateCombatStateManaScore(_initial_state, _state); // the less mana weve lost the better

	int64_t score = (attack_score << 48) | (defense_score << 32) | (turns_score << 16) | mana_score;
	return score;
}

int64_t CombatSimulator::evaluateCombatStateAttackScore(const CombatState& _initial_state, const CombatState& _state) const {
	float initial_fight_value = manager->getCombatAI().calculateBaseHeroFightValue(_initial_state.defender);
	float current_fight_value = manager->getCombatAI().calculateBaseHeroFightValue(_state.defender);

	return (1 << 15) * (1.0f - current_fight_value / initial_fight_value);
}

int64_t CombatSimulator::evaluateCombatStateDefenseScore(const CombatState& _initial_state, const CombatState& _state) const {
	float initial_fight_value = manager->getCombatAI().calculateBaseHeroFightValue(_initial_state.attacker);
	float current_fight_value = manager->getCombatAI().calculateBaseHeroFightValue(_state.attacker);

	return (1 << 15) * (current_fight_value / initial_fight_value);
}

int64_t CombatSimulator::evaluateCombatStateTurnsScore(const CombatState& _initial_state, const CombatState& _state) const {
	return (1 << 15) * (1.0f - (_state.turn - _initial_state.turn) / 250.0f);
}

int64_t CombatSimulator::evaluateCombatStateManaScore(const CombatState& _initial_state, const CombatState& _state) const {
	bool has_mana = _initial_state.attacker.getMana();

	if (has_mana)
		return (1 << 15) * (1.0f - (_state.attacker.getMana() - _initial_state.attacker.getMana()) / _initial_state.attacker.getMana());
	return (1 << 15);
}

// TODO: make more permutations; for now dont change unit order
void CombatSimulator::findBestAttackerPermutations() {
	ArmyPermutation permutation;

	for (int i = 0; i < attacker->army.size(); ++i)
		permutation.permutations.push_back(UnitPermutation{ i, i, attacker->army[i].stack_number });

	permutations.push_back(permutation);
}

void CombatSimulator::setDefenderPermutation() {
	for (int i = 0; i < defender->army.size(); ++i)
		defender_permutation.permutations.push_back(UnitPermutation{ i, i, defender->army[i].stack_number });
}

void CombatSimulator::start() {
	for (int i = 0; i < 1 /* combat field templates */; ++i) {
		for (auto permutation : permutations) {
			prepareCombat(permutation, /*i*/ 2);
			
			//std::list<CombatState> states;
			//std::list<CombatAction> actions;
			int action_cnt = 0;

			while (!manager->isCombatFinished()) {
				std::cout << "Actions: " << action_cnt++ << " | Turns: " << manager->getCurrentState().turn << std::endl;

				if (manager->isUnitMove()) {
					auto actions = manager->isPlayerMove() ? manager->generateActionsForPlayer() : manager->generateActionsForAI();
					int choice = getRandomInt(0, actions.size() - 1);
					manager->nextStateByAction(actions[choice]);
					continue;
				}
				
				manager->nextState();
			}
		}
	}
}

void CombatSimulator::prepareCombat(const ArmyPermutation& permutation, const int _field_template) {
	auto combat_attacker = prepareCombatHero(*attacker, permutation, CombatSide::ATTACKER);
	auto combat_defender = prepareCombatHero(*defender, defender_permutation, CombatSide::DEFENDER);
	auto combat_field = prepareCombatField(_field_template);

	manager = std::make_unique<CombatManager>(*combat_attacker, *combat_defender, *combat_field, combat_type);
	manager->initialize();
}

std::shared_ptr<CombatField> CombatSimulator::prepareCombatField(const int _field_template) {
	auto combat_field = std::make_shared<CombatField>(field_type);
	auto combat_field_template = getCombatFieldTemplate(_field_template);
	combat_field->setTemplate(combat_field_template);
	return combat_field;
}

std::shared_ptr<CombatHero> CombatSimulator::prepareCombatHero(const Hero& hero_template, const ArmyPermutation& permutation, const CombatSide _side) {
	std::shared_ptr<CombatHero> hero = std::make_shared<CombatHero>(hero_template, permutation, _side);

	return hero;
}

void CombatSimulator::initialize() {
	findBestAttackerPermutations();
	setDefenderPermutation();
}