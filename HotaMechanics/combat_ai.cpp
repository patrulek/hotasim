#include "combat_ai.h"
#include "combat_hero.h"

#include "combat_calculator.h"
#include "combat_manager.h"
#include "combat_pathfinder.h"

#include <cmath>

namespace HotaMechanics {
	using namespace Calculator;

	CombatAI::CombatAI(const CombatManager& _combat_manager)
		: combat_manager(_combat_manager) {

		pathfinder = std::make_unique<CombatPathfinder>();
	}

	void CombatAI::calculateFightValueAdvantageOnHexes(const CombatUnit& _active_stack, const CombatHero& _enemy_hero, const CombatField& _field) {
		hexes_fight_value_gain = Calculator::calculateFightValueAdvantageOnHexes(_active_stack, _enemy_hero, _field, *pathfinder);
		need_recalculate_hexes = false;
	}

	// because of randomization which cant be mirrored in this project, this function can possibly return more
	// than one unit to attack (only if some specified conditions are met; for most cases there will be only one unit)
	const std::vector<int> CombatAI::chooseUnitToAttack(const CombatUnit& activeStack, const CombatHero& enemy_hero) const {
		if (enemy_hero.getUnits().size() == 1)
			return std::vector<int>{ enemy_hero.getUnits()[0]->getUnitId() }; // todo: to could not always be [0]

		int turns = 999;
		std::vector<int> unit_ids{};
		int fight_value_gain = 0;
		int min_fight_value_gain = 0;
		int max_fight_value_gain = 0;
		int distance = 999;

		for (auto unit : enemy_hero.getUnits()) {
			if (!unit->isAlive())
				continue;

			int unit_distance = pathfinder->distanceBetweenHexes(activeStack.getHex(), unit->getHex()) - 1;
			int unit_turns = unit_distance == 0 ? 1 : std::ceil((float)unit_distance / activeStack.getCombatStats().spd); // 1 = can attack; todo: check if should be distance - 1
			int unit_fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(activeStack, *unit);

			// if no unit picked, pick first
			if (unit_ids.empty()) {
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = 0.75f * fight_value_gain;
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if less turns to attack, pick this one
			if (unit_turns < turns) {
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = 0.75f * fight_value_gain;
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if more turns to attack, check next
			if (unit_turns > turns)
				continue;

			// todo: this is simple randomization example; we'll checking fight_value range from 75%-100% of unit_fight_value_gain
			// if attacking current unit is better, then replace 
			if (unit_fight_value_gain * 0.75f > max_fight_value_gain) { // if cur_min_fv > ch_max_fv -> pick cur
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = 0.75f * fight_value_gain;
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if attacking chosen unit is better, check next
			if (unit_fight_value_gain < min_fight_value_gain) // if cur_max_fv < ch_min_fv -> pick ch
				continue;

			// fight_value_gain ranges for both units collide, so we need to add current unit to set
			// and update set fight_value_gain ranges
			unit_ids.push_back(unit->getUnitId());
			min_fight_value_gain = std::min(min_fight_value_gain, (int)(0.75f * fight_value_gain));
			max_fight_value_gain = std::max(max_fight_value_gain, fight_value_gain);

			/*
			* That is how it looks like in H3 code, but as we cant mirror here pseudorandomness
			* we introduce randomness in choosing unit, not calculating its final fight_value_gain
			*
			// if current unit has more health lost than pick current
			if (unit->health_lost > enemy_hero.getActiveUnits()[unit_ids[0]]->health_lost) {
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if current unit has less hp lost then go next
			if (unit->health_lost < enemy_hero.getActiveUnits()[unit_ids[0]]->health_lost)
				continue;

			// if current unit is closer then pick current
			if (unit_distance < distance) {
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if current unit is farther then go next
			if (unit_distance > distance)
				continue;

			// if equal then pick current
			unit_ids.push_back(unit->getUnitId());
			turns = unit_turns;
			fight_value_gain = unit_fight_value_gain;
			distance = unit_distance;
			*/
		}

		return unit_ids;
	}

	const int CombatAI::chooseHexToMoveForAttack(const CombatUnit& activeStack, const CombatUnit& target_unit) const {
		auto adjacent_hexes = pathfinder->getAdjacentHexesClockwise(target_unit.getHex());

		int hex = -1;
		int turns = -1;
		int hex_fight_value_gain = 0;
		int distance = -1;

		for (auto adj_hex : adjacent_hexes) {
			if (adj_hex != activeStack.getHex() && !combat_manager.getCurrentState().field.isHexWalkable(adj_hex))
				continue;

			int adj_distance = pathfinder->findPath(activeStack.getHex(), adj_hex, combat_manager.getCurrentState().field).size();
			int adj_turns = adj_distance == 0 ? 1 : std::ceil((float)adj_distance / activeStack.getCombatStats().spd);
			int adj_hex_fight_value_gain = hexes_fight_value_gain[adj_hex];

			// if didnt choose any yet, check first possible
			if (hex == -1) {
				hex = adj_hex;
				turns = adj_turns;
				hex_fight_value_gain = adj_hex_fight_value_gain;
				distance = adj_distance;
				continue;
			}

			// if current hex is more turns away from us than already chosen, go next
			if (adj_turns > turns)
				continue;

			// if current hex is worse in fight_value_gain terms, go next
			if (adj_hex_fight_value_gain < hex_fight_value_gain)
				continue;
			// if is better then choose this field
			else if (adj_hex_fight_value_gain > hex_fight_value_gain) {
				hex = adj_hex;
				turns = adj_turns;
				hex_fight_value_gain = adj_hex_fight_value_gain;
				distance = adj_distance;
				continue;
			}

			// if equal, check if we are unit that benefits from longer distances
			if (false /*TODO: check if cavalier or champion*/) {
				if (adj_distance > distance) {
					hex = adj_hex;
					turns = adj_turns;
					distance = adj_distance;
				}

				continue;
			}

			// if not, just choose field that is closer
			if (adj_distance < distance) {
				hex = adj_hex;
				distance = adj_distance;
			}
		}

		return hex;
	}


	const int CombatAI::chooseWalkDistanceFromPath(const CombatUnit& _active_stack, const int _target_hex, const CombatField& _field) const {
		int walk_distance = 0;
		int fight_value_gain = hexes_fight_value_gain[_active_stack.getHex()];
		auto path = pathfinder->findPath(_active_stack.getHex(), _target_hex, _field);
		int range = std::min(path.size(), (size_t)_active_stack.getCombatStats().spd);

		for (int i = 0; i < range; ++i) {
			if (hexes_fight_value_gain[path[i]] >= fight_value_gain) {
				fight_value_gain = hexes_fight_value_gain[path[i]];
				walk_distance = i + 1;
			}
		}

		return walk_distance;
	}
};

