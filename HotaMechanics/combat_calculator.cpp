#include "combat_calculator.h"

#include "combat_unit.h"
#include "combat_hero.h"
#include "combat_field.h"
#include "combat_pathfinder.h"

namespace HotaMechanics {
	using namespace Constants;

	namespace Calculator {
		const int calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) {
			int attacker_base_dmg = static_cast<int>(attacker.getBaseAverageDmg() * attacker.getStackNumber());
			float attacker_bonus_dmg = static_cast<float>(attacker_base_dmg);

			int attack_advantage = attacker.getBaseStats().atk - defender.getBaseStats().def;
			attacker_bonus_dmg *= (((attack_advantage > 0) * 0.05f) + ((attack_advantage < 0) * 0.025f)) * attack_advantage;

			int defender_total_health = defender.getStackNumber() * defender.getPrimaryStats().hp - defender.getHealthLost();

			// cant do more dmg than total health of defender
			return std::min((int)(attacker_base_dmg + attacker_bonus_dmg), defender_total_health);
		}

		const int calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) {
			if (!defender.canRetaliate())
				return 0;

			CombatUnit attacker_copy(attacker);
			CombatUnit defender_copy(defender);

			int first_attack_dmg = calculateMeleeUnitAverageDamage(attacker_copy, defender_copy);
			defender_copy.applyDamage(first_attack_dmg);

			if (!defender_copy.isAlive())
				return 0;

			int counter_attack_dmg = calculateMeleeUnitAverageDamage(defender_copy, attacker_copy);

			return counter_attack_dmg;
		}


		const float calculateUnitAttackFightValueModifier(const CombatUnit& unit) {
			// if has special abilities
			// if has special spells active
			return 1.0f + 0.05f * unit.getAttackGain();
		}

		const float calculateUnitDefenceFightValueModifier(const CombatUnit& unit) {
			// if has special abilities active
			// if has special spells active
			return 1.0f + 0.05f * unit.getDefenseGain();
		}

		const float calculateUnitFightValueModifier(const CombatUnit& unit) {
			return sqrt(calculateUnitAttackFightValueModifier(unit) * calculateUnitDefenceFightValueModifier(unit));
		}


		const int calculateStackUnitFightValue(const CombatUnit& unit) {
			float stack_modifier = calculateUnitFightValueModifier(unit);
			float stack_fight_value = unit.getFightValuePerUnitStack();

			return static_cast<int>(stack_modifier * stack_fight_value);
		}

		const int calculateBaseHeroFightValue(const CombatHero& hero) {
			return calculateHeroFightValue(hero); // TODO: for now its the same, but later need to modify this
		}

		const int calculateHeroFightValue(const CombatHero& hero) {
			int hero_fight_value = 0;

			for (auto unit : hero.getUnitsPtrs())
				hero_fight_value += calculateStackUnitFightValue(*unit);

			return hero_fight_value;
		}

		const int calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender) {
			CombatUnit attacker_copy(attacker);
			CombatUnit defender_copy(defender);

			int first_attack_dmg = calculateMeleeUnitAverageDamage(attacker_copy, defender_copy);
			defender_copy.applyDamage(first_attack_dmg);

			int counter_attack_dmg = calculateMeleeUnitAverageDamage(defender_copy, attacker_copy);
			attacker_copy.applyDamage(counter_attack_dmg);

			// TODO: when one side fight value > 2 * second side fight value -> then * 1000 for stronger side, and * 100 for weaker side
			int attacker_fight_value_gain = static_cast<int>(first_attack_dmg * defender_copy.getFightValuePerOneHp());
			int defender_fight_value_gain = static_cast<int>(counter_attack_dmg * attacker_copy.getFightValuePerOneHp());

			return attacker_fight_value_gain - defender_fight_value_gain;
		}

		const std::array<int, FIELD_SIZE> calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero, const CombatField& _field, const CombatPathfinder& _pathfinder) {
			std::array<int, FIELD_SIZE> hexes_fight_value;
			hexes_fight_value.fill(0);
			int max_fight_value_gain = calculateStackUnitFightValue(activeStack);
			const_cast<CombatPathfinder&>(_pathfinder).storePathCache();

			for (auto unit : enemy_hero.getUnitsPtrs()) {
				const_cast<CombatPathfinder&>(_pathfinder).clearPathCache();
				int fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, activeStack);

				if (fight_value_gain <= 0)
					continue;

				auto reachable_hexes = const_cast<CombatPathfinder&>(_pathfinder).getReachableHexesInRange(unit->getHex(), unit->getCombatStats().spd + 1, _field, false, false);
				reachable_hexes.push_back(unit->getHex()); // add also unit position

				for (auto hex : reachable_hexes) {
					int hex_fight_value_gain = std::min(max_fight_value_gain, fight_value_gain);
					hexes_fight_value[hex] = std::max(hexes_fight_value[hex], hex_fight_value_gain);
				}
			}

			const_cast<CombatPathfinder&>(_pathfinder).restorePathCache();
			std::for_each(std::begin(hexes_fight_value), std::end(hexes_fight_value), [](auto& obj) { obj = -obj; });
			return hexes_fight_value;
		}
	}; // HotaMechanics::Calculator
}; // HotaMechanics

