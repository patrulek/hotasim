#include "combat_estimator.h"

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/combat_calculator.h"

#include <numeric>
#include <random>
#include <algorithm>

using namespace HotaMechanics;
using namespace HotaMechanics::Constants;
using namespace HotaMechanics::Calculator;

namespace HotaSim {
	namespace Estimator
	{
		const int estimateTurnsNumber(const CombatState& _initial_state) {
			return 4; // TODO: implement
		}

		const int estimateTotalStatesNumber(const CombatState& _initial_state) {
			return 50000; // TODO: implement
		}

		const int estimateActionEffectivness(const CombatAction& _action, const CombatManager& _mgr) {
			auto& state = _mgr.getCurrentState();
			auto& active_stack = _mgr.getActiveStack();
			auto& ai = _mgr.getCombatAI();

			if (_action.action == CombatActionType::ATTACK) {
				auto& unit = _mgr.getStackByLocalId(_action.param1, CombatSide::DEFENDER);
				int score = 0;
				float i = 0.0f; float j = 0.0f;

				for (auto guid : state.order) {
					auto& unit_order = _mgr.getStackByGlobalId(guid);
					if (unit_order.getCombatSide() == CombatSide::ATTACKER) {
						auto adjacent = ai.getPathfinder().getAdjacentHexes(unit.getHex());
						j += 1.0f;

						for (auto adj_hex : adjacent) {
							if (!state.field.isHexWalkable(adj_hex))
								continue;
							if (const_cast<CombatAI&>(ai).canUnitAttackHex(unit_order, adj_hex)) {
								auto unit_it = std::find(std::begin(state.order), std::end(state.order), unit.getGlobalUnitId());
								auto unit_order_it = std::find(std::begin(state.order), std::end(state.order), guid);

								if (std::distance(std::begin(state.order), unit_order_it) < std::distance(std::begin(state.order), unit_it)
									|| unit_it == std::end(state.order)) {
									score += 50000;
									i += 1.0f;
								}
							}
						}
					}
				}

				score *= (i / j);
				
				if (unit.canRetaliate()) {
					int fv_gain = Calculator::calculateFightValueAdvantageAfterMeleeUnitAttack(active_stack, unit);
					score += fv_gain;
				}

				auto adjacent = ai.getPathfinder().getAdjacentHexes(unit.getHex());
				for (auto hex : adjacent) {
					if (!state.field.isHexWalkable(hex))
						continue;

					for (auto uu : _mgr.getCurrentState().defender.getUnitsPtrs()) {
						if (uu->getUnitId() == unit.getUnitId())
							continue;

						if (const_cast<CombatAI&>(ai).canUnitAttackHex(*uu, hex) && std::find(std::begin(state.order), std::end(state.order), uu->getGlobalUnitId()) != std::end(state.order)) {
							auto gunit = _mgr.getStackByLocalId(uu->getUnitId(), CombatSide::DEFENDER);
							int fv_gain = Calculator::calculateFightValueAdvantageAfterMeleeUnitAttack(active_stack, unit);
							score -= fv_gain;
						}
					}
				}

				return score;
			}
			if (_action.action == CombatActionType::DEFENSE)
				return 1;
			if (_action.action == CombatActionType::WAIT) {
				int len = 999;
				const CombatUnit* u{ nullptr };
				for (auto unit : state.defender.getUnitsPtrs()) {
					auto path = const_cast<CombatPathfinder&>(_mgr.getCombatAI().getPathfinder()).findPath(active_stack.getHex(), unit->getHex(), state.field, false, true);
					if (path.size() < len) {
						len = path.size();
						u = unit;
					}
				}

				int score = 0;
				auto u_it = std::find(std::begin(state.order), std::end(state.order), u->getGlobalUnitId());
				if(u_it != std::end(state.order))
					score = 20000;

				for (auto unit : state.attacker.getUnitsPtrs()) {
					if (unit == &active_stack)
						continue;

					auto unit_it = std::find(std::begin(state.order), std::end(state.order), unit->getGlobalUnitId());
					if (unit_it != std::end(state.order)) {
						if( std::distance(std::begin(state.order), unit_it) < std::distance(std::begin(state.order), u_it) || !unit->canWait())
							score += 20000;
					}
				}

				if (len > active_stack.getCombatStats().spd + 1) {
					if (len <= active_stack.getCombatStats().spd + u->getCombatStats().spd + 1)
						return 29000 + score;
					else
						return 1;
				}
				
				return 1;
			}
			if (_action.action == CombatActionType::WALK) {
				int summed_dist = 0;
				int score = 30000;

				

				for (auto unit : state.defender.getUnitsPtrs()) {
					if (const_cast<CombatAI&>(ai).canUnitAttackHex(*unit, _action.target)) {
						score -= Calculator::calculateStackUnitFightValue(*unit);
						score -= Calculator::calculateStackUnitFightValue(active_stack);
					}

					for (auto attacker_unit : state.attacker.getUnitsPtrs()) {
						if (const_cast<CombatAI&>(ai).canUnitAttackHex(*unit, attacker_unit->getHex())) {
							score -= Calculator::calculateStackUnitFightValue(*unit);
							score -= Calculator::calculateStackUnitFightValue(*attacker_unit);
						}
					}
					//auto path = const_cast<CombatPathfinder&>(_mgr.getCombatAI().getPathfinder()).findPath(unit->getHex(), _action.target, state.field, false, unit->getCombatStats().spd);
					summed_dist += _mgr.getCombatAI().getPathfinder().distanceBetweenHexes(unit->getHex(), _action.target);//path.size();
				}

				score -= 1000 * summed_dist;
				return score;
			}

			return 1;
		}

		const std::vector<int> shuffleActions(const std::vector<CombatAction>& _actions, const CombatManager& _manager, const int _seed) {
			std::vector<int> action_order(_actions.size());
			std::iota(std::begin(action_order), std::end(action_order), 0);
			std::vector<int> action_effectivness;

			for (int i = 0; i < _actions.size(); ++i)
				action_effectivness.push_back(estimateActionEffectivness(_actions[i], _manager));

			if (_seed == 42) {
				std::sort(std::begin(action_order), std::end(action_order), [&action_effectivness](auto idx, auto idx2) {
					return action_effectivness[idx] > action_effectivness[idx2];
				});
			}
			else {
				auto seed = std::seed_seq{ _seed };
				std::shuffle(std::begin(action_order), std::end(action_order), std::mt19937(seed));
			}
			return action_order;
		}
	}; // HotaSim::Estimator
} // HotaSim

