#include "combat_simulator.h"

#include "../HotaMechanics/combat_hero.h"
#include "../HotaMechanics/combat_field.h"

CombatSimulator::CombatSimulator(const Hero& _attacker, const Hero& _defender, 
											const CombatFieldType _field_type, const CombatType _combat_type)
	: field_type(_field_type), combat_type(_combat_type) {
	
	attacker = std::make_unique<Hero>(_attacker);
	defender = std::make_unique<Hero>(_defender);
}

CombatSimulator::~CombatSimulator() {}

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
			prepareCombat(permutation, i);
		}
	}
}

void CombatSimulator::prepareCombat(const ArmyPermutation& permutation, const int _field_template) {
	auto combat_attacker = prepareCombatHero(*attacker, permutation);
	auto combat_defender = prepareCombatHero(*defender, defender_permutation);
	auto combat_field = prepareCombatField(_field_template);

	manager = std::make_unique<CombatManager>(combat_attacker, combat_defender, combat_field);
}

std::shared_ptr<CombatField> CombatSimulator::prepareCombatField(const int _field_template) {
	auto combat_field = std::make_shared<CombatField>(field_type);
	auto combat_field_template = getCombatFieldTemplate(_field_template);
	combat_field->setTemplate(combat_field_template);
	return combat_field;
}

std::shared_ptr<CombatHero> CombatSimulator::prepareCombatHero(const Hero& hero_template, const ArmyPermutation& permutation) {
	std::shared_ptr<CombatHero> hero = std::make_shared<CombatHero>(hero_template);

	return hero;
}

void CombatSimulator::initialize() {
	findBestAttackerPermutations();
	setDefenderPermutation();
}