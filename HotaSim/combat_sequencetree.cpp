#include "combat_sequencetree.h"

#include "../HotaMechanics/combat_manager.h"
#include "combat_manager_serializer.h"

#include "mempool.h"

using namespace HotaMechanics;

namespace HotaSim {

	CombatSequenceTree::CombatSequenceTree(const CombatSerializer& _serializer, const HotaMechanics::CombatManager& _manager, const CombatState& _initial_state, const uint64_t _initial_state_score)
		: serializer(_serializer), manager(_manager) {
		const auto& packed_state = const_cast<CombatSerializer&>(serializer).packCombatState(_initial_state);
		size = 1;

		auto& root_node = Mempool::retrieveCombatSequenceNode();
		root_node->state = packed_state;
		root_node->action = 0;
		root_node->action_size = 1;
		root_node->score = root_node->best_branch_score = _initial_state_score;
		root_node->parent = nullptr;
		root_node->seed = 1;
		
		root = root_node;
		current = root.get();
		forgotten_paths.reserve(8096);
		state_hashes.rehash(262144);
		turns_occurence.fill(0);
		level_occurence.fill(0);
	}



	void CombatSequenceNode::addChild(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const uint64_t _size) {
		score = _state_score;

		CombatSequenceNode* branch = this;
		int new_depth = children.empty();

		while (branch) {
			if (_state_score > branch->best_branch_score)
				branch->best_branch_score = _state_score;

			branch->depth = std::max(branch->depth, branch->depth + new_depth);
			branch = branch->parent;
		}

		auto& child_node = Mempool::retrieveCombatSequenceNode();
		child_node->state = _state;
		child_node->action = _action;
		child_node->action_size = _action_size;
		child_node->score = child_node->best_branch_score = _state_score;
		child_node->parent = this;
		child_node->seed = _seed;
		child_node->level = this->level + 1;
		child_node->id = _size + 1;
		if (child_node->action_size > 1)
			children.reserve(32);
		children.emplace_back(std::move(child_node));
	}


	void CombatSequenceTree::addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const bool _first_ai_attack) {
		circular_path_found = false;

		if (best_score & 0x8000000000000000) {
			if ((_state_score & 0x0000FFFFFFFFFFFF) < (best_score & 0x0000FFFFFFFFFFFF) && _state.turn >= best_turns) {
				++early_cutoff;
				circular_path_found = true;
				return;
			}
		}

		if (_first_ai_attack) {
			++early_cutoff;
			circular_path_found = true;
			return;
		}

		auto state_hash = StateHash(const_cast<CombatState&>(_state));

		if (auto it = state_hashes.find(state_hash); it != std::end(state_hashes)
			&& _state.turn >= it->second && _state.order.empty()) {
			//auto it2 = node_hashes.find(StateHash(const_cast<CombatState&>(_state)));
			//auto hash = StateHash(const_cast<CombatState&>(_state));
			circular_path_found = true;
			++circular_occurence;
			return;
		} else {
			//state_hashes.insert(StateHash(const_cast<CombatState&>(_state)));
			state_hashes[state_hash] = _state.turn;
		}

		if (_state_score > best_score) {
			best_score = _state_score;
			best_turns = _state.turn;
		}

		++level_occurence[current->level];
		++size;
		++turns_occurence[_state.turn];
		auto packed_state = const_cast<CombatSerializer&>(serializer).packCombatState(_state);
		current->addChild(packed_state, _action, _action_size, _state_score, _seed, size);
		//node_hashes[StateHash(const_cast<CombatState&>(_state))] = current->children.back().get();
		current = current->children.back().get();
	}

	void CombatSequenceTree::sortForgotten() {
		std::sort(std::begin(forgotten_paths) + fp_cnt, std::end(forgotten_paths), [](auto _node1, auto _node2) { return _node1->best_branch_score > _node2->best_branch_score; });
		since_last_sort = 0;
	}

	void CombatSequenceTree::takeForgotten() {
		if (since_last_sort > 500)
			sortForgotten();

		if (!canTakeForgotten()) {
			return;
		}

		current = forgotten_paths[fp_cnt++];
	}

	void CombatSequenceTree::goParent() {
		if (!current->parent)
			throw std::exception("Should never happen (cant go to null parent node)");

		current = current->parent;
	}

	void CombatSequenceTree::goRandomParent(const bool _combat_finished) {
		if (current->level < 6) {
			goRoot(_combat_finished);
			return;
		}

		if (!_combat_finished)
			forgotten_paths.push_back(current);

		int target_turn = 2; // make random > 1

		// go to the first state of a given turn
		while (current->state->turn > target_turn)
			goParent();

		// go to first not checked state from this turn
		while (!current->children.empty() && current->children.back()->action_size == current->children.back()->action + 1) {
			current = current->children.back().get();
		}
	}

	void CombatSequenceTree::goRoot(const bool _combat_finished) {
		if (!_combat_finished)
			forgotten_paths.push_back(current);


		current = root.get();

		while (!current->children.empty() && current->children.back()->action_size == current->children.back()->action + 1) {
			current = current->children.back().get();
		}

		// checked all states, go root and end simulation
		if (current->children.empty() && current->action_size == current->action + 1) {
			current = root.get();
			return;
		}
	}

	CombatSequenceNode* CombatSequenceTree::findBestLeaf() {
		CombatSequenceNode* branch = root.get();

		while (!branch->children.empty()) {
			int best_depth = 999;
			int best_idx = 0, idx = 0;
			uint64_t best_score = 0x0000000000000000;

			for (auto child : branch->children) {
				if (child->best_branch_score == branch->best_branch_score && child->score > best_score) {
					best_idx = idx;
					best_score = child->score;
				}
				++idx;
			}

			branch = branch->children[best_idx].get();
		}

		return branch;
	}
}