#include "combat_ai.h"
#include "combat_hero.h"

#include "combat_calculator.h"
#include "combat_manager.h"
#include "combat_pathfinder.h"

#include <cmath>

namespace HotaMechanics {
	using namespace Calculator;
	using namespace Constants;

	CombatAI::CombatAI(const CombatManager& _combat_manager)
		: combat_manager(_combat_manager) {

		pathfinder = std::make_unique<CombatPathfinder>();
		events_to_process.reserve(128);
	}

	std::vector<int16_t> CombatAI::getReachableHexesForUnit(const CombatUnit& _unit) const {
		std::vector<int16_t> reachable; reachable.reserve(64);
		auto& reachables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;

		for (int hex = 0, uid = _unit.getUnitId(); hex < FIELD_SIZE; ++hex) {
			if (reachables[hex] & (1 << uid))
				reachable.push_back(hex);
		}

		return reachable;
	}

	std::vector<int16_t> CombatAI::getAttackableHexesForUnit(const CombatUnit& _unit) const {
		std::vector<int16_t> attackable; attackable.reserve(64);
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;

		for (int hex = 0, uid = _unit.getUnitId(); hex < FIELD_SIZE; ++hex) {
			if (attackables[hex] & (1 << uid))
				attackable.push_back(hex);
		}

		return attackable;
	}

	const bool CombatAI::canUnitReachHex(const CombatUnit& _unit, const int16_t _hex) const {
		auto& reachables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
		auto uid = _unit.getUnitId();

		return reachables[_hex] & (1 << uid);
	}

	const bool CombatAI::isHexInUnitRange(const CombatUnit& _unit, const int16_t _hex) const {
		auto& ranges = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_ranges : ai_unit_ranges;
		auto uid = _unit.getUnitId();

		return ranges[_hex] & (1 << uid);
	}

	const bool CombatAI::canUnitAttackHex(const CombatUnit& _unit, const int16_t _hex) const {
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
		auto uid = _unit.getUnitId();

		return attackables[_hex] & (1 << uid);
	}

	const FieldArray& CombatAI::getAttackablesForUnit(const CombatUnit& _unit) const {
		return _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
	}

	const FieldArray& CombatAI::getReachablesForUnit(const CombatUnit& _unit) const {
		return _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
	}

	std::vector<int16_t> CombatAI::getAttackableHexesForUnitFromList(const CombatUnit& _unit, std::vector<int16_t>& _hexes) const {
		std::vector<int16_t> attackable; attackable.reserve(64);
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
		int uid = _unit.getUnitId();

		for (auto hex : _hexes) {
			if (attackables[hex] & (1 << uid))
				attackable.push_back(hex);
		}

		return attackable;
	}

	std::vector<const CombatUnit*> CombatAI::getEnemyUnitsInAttackRange(const CombatUnit& _unit) const {
		std::vector<const CombatUnit*> units; units.reserve(16);
		auto& hero = _unit.getCombatSide() == CombatSide::ATTACKER ? combat_manager.getCurrentState().defender : combat_manager.getCurrentState().attacker;
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
		int uid = _unit.getUnitId();

		for (auto enemy_unit : hero.getUnitsPtrs()) {
			if (!enemy_unit->isAlive())
				continue;
			
			if( canUnitAttackHex(_unit, enemy_unit->getHex()))
				units.push_back(enemy_unit);
		}

		return units;
	}

	void CombatAI::initializeBattle(const FieldArray* _player_unit_reachables, const FieldArray* _player_unit_attackables,
											  const FieldArray* _ai_unit_reachables, const FieldArray* _ai_unit_attackables, const bool _only_ranges) {
		initializePlayerUnitRanges();
		initializeAIUnitRanges(); 

		if (_only_ranges) {
			// temp - check correctness
			FieldArray current_reachables = player_unit_reachables;
			initializePlayerUnitAttackables();
			for (int i = 0; i < FIELD_SIZE + 1; ++i) {
				if (current_reachables[i] != player_unit_reachables[i]) {
					throw std::exception("xxx, never happen");
				}
			}
			//
			return;
		}

		initializePlayerUnitAttackables(_player_unit_reachables, _player_unit_attackables);
		initializeAIUnitAttackables(_ai_unit_reachables, _ai_unit_attackables);
	}

	void CombatAI::initializePlayerUnitRanges() {
		for (auto unit : combat_manager.getCurrentState().attacker.getUnitsPtrs()) {
			clearUnitRanges(*unit);
			setUnitRanges(*unit);
		}
	}

	void CombatAI::initializePlayerUnitAttackables(const FieldArray* _player_unit_reachables, const FieldArray* _player_unit_attackables) {
		if (_player_unit_reachables) {
			for (int16_t hex = 0; hex < FIELD_SIZE + 1; ++hex) {
				player_unit_reachables[hex] = (*_player_unit_reachables)[hex];
				player_unit_attackables[hex] = (*_player_unit_attackables)[hex];
			}
			return;
		}

		for (auto unit : combat_manager.getCurrentState().attacker.getUnitsPtrs()) {
			clearUnitAttackables(*unit);
			setUnitAttackables(*unit);
			pathfinder->clearPathCache();
		}
	}

	void CombatAI::initializeAIUnitRanges() {
		for (auto unit : combat_manager.getCurrentState().defender.getUnitsPtrs()) {
			clearUnitRanges(*unit);
			setUnitRanges(*unit);
		}
	}

	void CombatAI::initializeAIUnitAttackables(const FieldArray* _ai_unit_reachables, const FieldArray* _ai_unit_attackables) {
		if (_ai_unit_reachables) {
			for (int16_t hex = 0; hex < FIELD_SIZE + 1; ++hex) {
				ai_unit_reachables[hex] = (*_ai_unit_reachables)[hex];
				ai_unit_attackables[hex] = (*_ai_unit_attackables)[hex];
			}
			return;
		}

		for (auto unit : combat_manager.getCurrentState().defender.getUnitsPtrs()) {
			clearUnitAttackables(*unit);
			setUnitAttackables(*unit);
			pathfinder->clearPathCache();
		}
	}

	void CombatAI::processEvents() {
		for (auto& ev : events_to_process) {
			if (ev.type == CombatEventType::UNIT_POS_CHANGED) {
				pathfinder->clearPathCache();
				processUnitPosChangedEvent(ev);
			}
			else if (ev.type == CombatEventType::UNIT_HEALTH_LOST)
				processUnitHealthLostEvent(ev);
		}

		events_to_process.clear();
		pathfinder->clearPathCache();
	}

	void CombatAI::processUnitPosChangedEvent(const CombatEvent& _ev) {
		auto& unit = combat_manager.getStackByGlobalId(_ev.param1);
		auto& field = combat_manager.getCurrentState().field;
		//combat_manager.getCurrentState().field.rehash();

		clearUnitRanges(unit);
		clearUnitAttackables(unit);
		setUnitRanges(unit);
		setUnitAttackables(unit);

		for (auto u : combat_manager.getAllUnitStacks()) {
			if (u == &unit)
				continue;

			pathfinder->clearPathCache();
			auto unit_bit = (1 << u->getUnitId());
			if (canUnitAttackHex(*u, _ev.param2) || canUnitReachHex(*u, _ev.param3)) {
				clearUnitAttackables(*u);
				setUnitAttackables(*u);
			}
			//auto& reachables = u->getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
			//auto& attackables = u->getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
			//
			//// if old unit pos is in range and we could attack this hex
			//if (isHexInUnitRange(*u, _ev.param2) && canUnitAttackHex(*u, _ev.param2)) {
			//	// set old unit pos as reachable now
			//	reachables[unit.getHex()] |= unit_bit;
			//	for (auto adj_hex : pathfinder->getAdjacentHexes(unit.getHex()))
			//		attackables[adj_hex] |= unit_bit;

			//	// go through all hexes in range that we could not reach earlier, and check that we can reach it now
			//	for (auto range_hex : pathfinder->getHexesInRange(u->getHex(), u->getCombatStats().spd)) {
			//		if (range_hex == u->getHex())
			//			continue;

			//		if (!canUnitReachHex(*u, range_hex) && pathfinder->realDistanceBetweenHexes(u->getHex(), range_hex, field, false) <= u->getCombatStats().spd) {
			//			reachables[range_hex] |= unit_bit;
			//			attackables[range_hex] |= unit_bit;

			//			for (auto adj_hex : pathfinder->getAdjacentHexes(range_hex))
			//				attackables[adj_hex] |= unit_bit;
			//		}
			//	}
			//}

			//// if new unit pos is in range and was reachable
			//if (isHexInUnitRange(*u, _ev.param3) && canUnitReachHex(*u, _ev.param3)) {
			//	std::vector<int16_t> to_check; to_check.reserve(64);
			//	to_check.push_back(_ev.param3);

			//	// check recursively adjacent hexes to that pos to check if reachiness of adj hex changes
			//	for (auto check_hex : to_check) {
			//		int size = to_check.size();
			//		reachables[_ev.param3] &= (~unit_bit);

			//		for (auto adj_hex : pathfinder->getAdjacentHexes(check_hex)) {
			//			if (canUnitReachHex(*u, adj_hex) && pathfinder->realDistanceBetweenHexes(u->getHex(), adj_hex, field, false) > u->getCombatStats().spd) {
			//				reachables[adj_hex] &= (~unit_bit);
			//				to_check.push_back(adj_hex);
			//			}
			//		}
			//	}

			//	// go through all hexes in range that we could reach earlier, and check that we cannot reach it now
			//	for (auto range_hex : pathfinder->getHexesInRange(u->getHex(), u->getCombatStats().spd)) {
			//		if (canUnitReachHex(*u, range_hex) && pathfinder->realDistanceBetweenHexes(u->getHex(), range_hex, field, false) > u->getCombatStats().spd) {
			//			reachables[range_hex] &= (~unit_bit);
			//		}
			//	}
			//}


			//// now reset all attackables regarding new reachables
			//for (auto range_hex : pathfinder->getHexesInRange(u->getHex(), u->getCombatStats().spd + 1))
			//	attackables[range_hex] &= (~unit_bit);

			//attackables[u->getHex()] |= unit_bit;
			//for (auto adj_hex : pathfinder->getAdjacentHexes(u->getHex()))
			//	attackables[adj_hex] |= unit_bit;

			//for (auto range_hex : pathfinder->getHexesInRange(u->getHex(), u->getCombatStats().spd)) {
			//	if (canUnitReachHex(*u, range_hex)) {
			//		for (auto adj_hex : pathfinder->getAdjacentHexes(range_hex)) {
			//			attackables[adj_hex] |= unit_bit;
			//		}
			//	}
			//}
		}
	}
	
	void CombatAI::setUnitRanges(const CombatUnit& _unit) {
		auto hexes = pathfinder->getHexesInRange(_unit.getHex(), _unit.getCombatStats().spd);
		auto& ranges = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_ranges : ai_unit_ranges;

		for (auto hex : hexes)
			ranges[hex] |= (1 << _unit.getUnitId());
	}

	void CombatAI::setUnitAttackables(const CombatUnit& _unit) {
		auto& adjacent_hexes = pathfinder->getAdjacentHexes(_unit.getHex());
		auto& reachables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;
		const int unit_bit = (1 << _unit.getUnitId());

		attackables[_unit.getHex()] |= unit_bit;

		for (auto adj_hex : adjacent_hexes)
			attackables[adj_hex] |= unit_bit;

		auto&& hexes = pathfinder->getReachableHexesInRange(_unit.getHex(), _unit.getCombatStats().spd, combat_manager.getCurrentState().field, false, false, false);
		for (auto hex : hexes) {
			reachables[hex] |= unit_bit;
			attackables[hex] |= unit_bit;

			auto& attackable_adjacent = pathfinder->getAdjacentHexes(hex);
			for (auto att_adj_hex : attackable_adjacent)
				attackables[att_adj_hex] |= unit_bit;
		}
	}

	void CombatAI::processUnitHealthLostEvent(const CombatEvent& _ev) {
		auto& unit = combat_manager.getStackByGlobalId(_ev.param1);

		if (unit.isAlive())
			return;

		//combat_manager.getCurrentState().field.rehash();
		clearUnitRanges(unit);
		clearUnitAttackables(unit);

		auto& field = combat_manager.getCurrentState().field;
		//std::vector<int16_t> to_check(64);

		for (auto u : combat_manager.getAllUnitStacks()) {
			if (u == &unit)
				continue;

			pathfinder->clearPathCache();
			// if died unit was in unit range and unit was attackable (so we could reach any adjacent hex to it)
			if (isHexInUnitRange(*u, unit.getHex()) && canUnitAttackHex(*u, unit.getHex())) {
				auto unit_bit = (1 << u->getUnitId());
				auto& reachables = u->getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
				auto& attackables = u->getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;

				// set dead unit pos as reachable now
				reachables[unit.getHex()] |= unit_bit;
				for (auto adj_hex : pathfinder->getAdjacentHexes(unit.getHex()))
					attackables[adj_hex] |= unit_bit;

				// go through all hexes in range that we could not reach earlier, and check that we can reach it now
				for (auto range_hex : pathfinder->getHexesInRange(u->getHex(), u->getCombatStats().spd)) {
					if (!canUnitReachHex(*u, range_hex) 
					&& pathfinder->realDistanceBetweenHexes(u->getHex(), range_hex, field, false, u->getCombatStats().spd) <= u->getCombatStats().spd) {
						reachables[range_hex] |= unit_bit;
						attackables[range_hex] |= unit_bit;

						for( auto adj_hex : pathfinder->getAdjacentHexes(range_hex))
							attackables[adj_hex] |= unit_bit;
					}
				}
			}
		}
	}

	void CombatAI::clearUnitRanges(const CombatUnit& _unit) {
		auto& ranges = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_ranges : ai_unit_ranges;

		for (int hex = 0, uid = _unit.getUnitId(); hex < FIELD_SIZE; ++hex) {
			ranges[hex] &= (~(1 << uid));
		}
	}

	void CombatAI::clearUnitAttackables(const CombatUnit& _unit) {
		auto& reachables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_reachables : ai_unit_reachables;
		auto& attackables = _unit.getCombatSide() == CombatSide::ATTACKER ? player_unit_attackables : ai_unit_attackables;

		for (int hex = 0, uid = _unit.getUnitId(); hex < FIELD_SIZE; ++hex) {
			attackables[hex] &= (~(1 << uid));
			reachables[hex] &= (~(1 << uid));
		}
	}

	void CombatAI::calculateFightValueAdvantageOnHexes(const CombatUnit& _active_stack, const CombatHero& _enemy_hero, const CombatField& _field) {
		int max_fight_value_gain = calculateStackUnitFightValue(_active_stack);
		hexes_fight_value_gain.fill(0);

		for (auto unit : const_cast<CombatHero&>(_enemy_hero).getUnitsPtrs()) {
			bool can_reach_any_unit = false;

			for (auto friendly_unit : _active_stack.getHero()->getUnitsPtrs()) { // h3hota hd.exe + 2055D
				can_reach_any_unit |= canUnitAttackHex(*unit, friendly_unit->getHex());
			}

			if( can_reach_any_unit )
				continue;

			int fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(*unit, _active_stack);

			if (fight_value_gain <= 0)
				continue;

			for (int hex = 0, uid = unit->getUnitId(); hex < FIELD_SIZE; ++hex) {
				if (player_unit_attackables[hex] & (1 << uid)) {
					hexes_fight_value_gain[hex] = std::min(max_fight_value_gain, hexes_fight_value_gain[hex] + fight_value_gain);
				}
			}
		}

		std::for_each(std::begin(hexes_fight_value_gain), std::end(hexes_fight_value_gain), [](auto& obj) { obj = -obj; });
	}

	const bool CombatAI::isHexBlockedFor(const int16_t _target_hex, const CombatUnit& _active_stack) const {
		auto& adjacent_hexes = pathfinder->getAdjacentHexes(_target_hex);
		const bool staying_around = (_target_hex == _active_stack.getHex()) ||
			std::any_of(std::begin(adjacent_hexes), std::end(adjacent_hexes), [&_active_stack](const auto adj_hex) {return _active_stack.getHex() == adj_hex; });

		if (staying_around)
			return false;

		bool hex_access = false;
		for (auto adj_hex : adjacent_hexes) {
			if (combat_manager.getCurrentState().field.isHexWalkable(adj_hex))
				hex_access = true;
			if (hex_access)
				break;
		}

		if (!hex_access)
			return true;

		// it should be better to check path from given hex to active stack than otherwise, because if hex is blocked it probably ends searching faster
		// const bool path_exists = !pathfinder->findPath(_target_hex, _active_stack.getHex(), combat_manager.getCurrentState().field, false, true).empty();
		const bool path_exists = !pathfinder->findPath(_active_stack.getHex(), _target_hex, combat_manager.getCurrentState().field, false, true).empty();
		return !path_exists;
	}

	const bool CombatAI::isUnitBlockedFor(const CombatUnit& _unit, const CombatUnit& _active_stack) const {
		return isHexBlockedFor(_unit.getHex(), _active_stack);
	}

	// because of randomization which cant be mirrored in this project, this function can possibly return more
	// than one unit to attack (only if some specified conditions are met; for most cases there will be only one unit)
	const std::vector<int> CombatAI::chooseUnitToAttack(const CombatUnit& _active_stack, const CombatHero& _enemy_hero, const std::vector<int16_t>& _hexes_to_attack) const {
		if (_enemy_hero.getUnitsPtrs().size() == 1)
			return std::vector<int>{ _enemy_hero.getUnitsPtrs()[0]->getUnitId() }; // todo: to could not always be [0]

		int turns = 999;
		std::vector<int> unit_ids{};
		int fight_value_gain = 0;
		int min_fight_value_gain = 0;
		int max_fight_value_gain = 0;
		int distance = 999;
		bool attackable = false;
		auto& field = combat_manager.getCurrentState().field;
		int cnt = -1;

		for (auto unit : _enemy_hero.getUnitsPtrs()) {
			++cnt;
			const bool hex_reachable = _hexes_to_attack[cnt] == _active_stack.getHex() || field.isHexWalkable(_hexes_to_attack[cnt]);
			const bool unit_attackable = true;// !isUnitBlockedFor(*unit, _active_stack);

			if (!unit->isAlive() || !unit_attackable || !hex_reachable)
				continue;

			const bool unit_hex = (_hexes_to_attack[cnt] == _active_stack.getHex());
			int unit_distance = unit_hex ? 0 : pathfinder->realDistanceBetweenHexes(_active_stack.getHex(), _hexes_to_attack[cnt], field, false);
			int unit_turns = static_cast<int>(unit_hex ? 1 : std::ceil((float)unit_distance / _active_stack.getCombatStats().spd)); // 1 = can attack; todo: check if should be distance - 1

			
			int unit_fight_value_gain = calculateFightValueAdvantageAfterMeleeUnitAttack(_active_stack, *unit); //351d0

			// if no unit picked or chosen unit not attackable
			if (unit_ids.empty()) {
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = static_cast<int>(0.75f * fight_value_gain);
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				attackable = unit_attackable;
				continue;
			}

			// if less turns to attack, pick this one
			if (unit_turns < turns) {
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = static_cast<int>(0.75f * fight_value_gain);
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if more turns to attack, check next
			if (unit_turns > turns)
				continue;

			// todo: this is simple randomization example; we'll checking fight_value range from 75%-100% of unit_fight_value_gain
			// if attacking current unit is better, then replace 
			if (static_cast<int>(unit_fight_value_gain * 0.75f) > max_fight_value_gain) { // if cur_min_fv > ch_max_fv -> pick cur
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				turns = unit_turns;
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = static_cast<int>(0.75f * fight_value_gain);
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
				continue;
			}

			// if attacking chosen unit is better, check next
			if (unit_fight_value_gain < min_fight_value_gain) // if cur_max_fv < ch_min_fv -> pick ch
				continue;

			
			// fight_value_gain ranges for both units collide, so we need to add current unit to set
			// and update set fight_value_gain ranges

			// check if closer
			if (unit_distance < distance) {
				unit_ids.clear();
				unit_ids.push_back(unit->getUnitId());
				fight_value_gain = unit_fight_value_gain / unit_turns;
				min_fight_value_gain = static_cast<int>(0.75f * fight_value_gain);
				max_fight_value_gain = fight_value_gain;
				distance = unit_distance;
			}

			if (unit_distance > distance)
				continue;

			unit_ids.push_back(unit->getUnitId());
			min_fight_value_gain = std::min(min_fight_value_gain, static_cast<int>(0.75f * fight_value_gain));
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

	const int16_t CombatAI::chooseHexToMoveForAttack(const CombatUnit& activeStack, const CombatUnit& target_unit) const {
		auto adjacent_hexes = pathfinder->getAdjacentHexesClockwise(target_unit.getHex());
		auto& field = combat_manager.getCurrentState().field;
		auto& pathfinder_ = *pathfinder;

		int hex = INVALID_HEX_ID;
		int turns = -1;
		int hex_fight_value_gain = 0;
		int distance = -1;

		for (auto adj_hex : adjacent_hexes) {
			if (isHexBlockedFor(adj_hex, activeStack))
				continue;

			int adj_distance = pathfinder->realDistanceBetweenHexes(activeStack.getHex(), adj_hex, combat_manager.getCurrentState().field);

			if (adj_distance == 0 && (adj_hex != activeStack.getHex())) // target hex isnt blocked but is not walkable
				continue;

			int adj_turns = static_cast<int>((adj_distance == 0) ? 1 : std::ceil((float)adj_distance / activeStack.getCombatStats().spd));
			int adj_hex_fight_value_gain = hexes_fight_value_gain[adj_hex];

			// if didnt choose any yet, check first possible
			if (hex == INVALID_HEX_ID) {
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


	const int CombatAI::chooseWalkDistanceFromPath(const CombatUnit& _active_stack, const int _target_hex, const CombatField& _field, const int _unit_id) const {
		int walk_distance = !_active_stack.canWait();
		auto path = pathfinder->findPath(_active_stack.getHex(), _target_hex, _field);

		if (path.empty()) {
			throw std::exception("Should never happen here (empty path)");
		}

		int fight_value_gain = !_active_stack.canWait() ? hexes_fight_value_gain[path[0]] : hexes_fight_value_gain[_active_stack.getHex()];
		int range = (int)std::min(path.size(), (size_t)_active_stack.getCombatStats().spd);

		for (int i = walk_distance; i < range; ++i) {
			if ( hexes_fight_value_gain[path[i]] >= fight_value_gain) {
				fight_value_gain = hexes_fight_value_gain[path[i]];
				walk_distance = i + 1;
			}

			if ( !_active_stack.canWait() && i + 1 >= walk_distance) {
				if (std::ceil((float)(path.size() - walk_distance) / _active_stack.getCombatStats().spd
					> std::ceil((float)(path.size() - i - 1) / _active_stack.getCombatStats().spd))) {
					walk_distance++;
					i = walk_distance - 1;
					fight_value_gain = hexes_fight_value_gain[path[walk_distance]];
				}
			}
		}

		return walk_distance;
	}
};

