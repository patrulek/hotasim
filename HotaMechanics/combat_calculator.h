#pragma once

#include <array>
#include "constants.h"

namespace HotaMechanics {
	class CombatUnit;
	class CombatHero;
	class CombatField;
	class CombatPathfinder;

	namespace Calculator {
		// calculate damage -------------
		const int calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender);
		const int calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender);
		// ------------------------------

		// calculate modifiers ----------
		const float calculateUnitAttackFightValueModifier(const CombatUnit& unit);
		const float calculateUnitDefenceFightValueModifier(const CombatUnit& unit);
		const float calculateUnitFightValueModifier(const CombatUnit& unit);
		// ------------------------------

		// calculate fight values -------
		const int calculateStackUnitFightValue(const CombatUnit& unit);
		const int calculateBaseHeroFightValue(const CombatHero& hero);
		const int calculateHeroFightValue(const CombatHero& hero);
		const int calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender, const bool _advantage = false);
		const std::array<int, Constants::FIELD_SIZE> calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero, const CombatField& _field, const CombatPathfinder& _pathfinder);
		// ------------------------------
	}; // HotaMechanics::Calculator
}; // HotaMechanics

