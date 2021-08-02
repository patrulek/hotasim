#pragma once

#include <memory>

#include "combat_permutation.h"

// for initialization
struct Hero;
enum class CombatFieldType;
enum class CombatType;

// for combat simulation
class CombatManager;
class CombatHero;

class CombatSimulator
{
public:
	CombatSimulator(const Hero& _attacker, const Hero& _defender, const CombatFieldType _field_type, const CombatType _combat_type);
	~CombatSimulator();

	void initialize();

	void start();
private:
	void findBestAttackerPermutations();
	void setDefenderPermutation();

	void prepareCombat(const ArmyPermutation& _permutation, const int _field_template);
	std::shared_ptr<CombatField> prepareCombatField(const int _field_template);
	std::shared_ptr<CombatHero> prepareCombatHero(const Hero& _hero_template, const ArmyPermutation& _permutation);

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

