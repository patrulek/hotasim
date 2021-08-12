#include "combat_sequencetree.h"

#include "../HotaMechanics/combat_manager.h"

using namespace HotaMechanics;

namespace HotaSim {

	CombatSequenceTree::CombatSequenceTree(const HotaMechanics::CombatManager& _manager, const CombatState& _initial_state, const uint64_t _initial_state_score)
		: manager(_manager) {
		auto packed_state = const_cast<CombatManager&>(manager).packCombatState(_initial_state);
		size = 1;

		root = std::make_shared<CombatSequenceNode>(packed_state, 0, 1, _initial_state_score, nullptr, 1);
		current = root.get();
		forgotten_paths.reserve(8096);
		turns_occurence.fill(0);
		level_occurence.fill(0);
	}



	void CombatSequenceNode::addChild(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const int _size) {
		score = _state_score;

		CombatSequenceNode* branch = this;
		int new_depth = children.empty();

		while (branch) {
			if (_state_score > branch->best_branch_score)
				branch->best_branch_score = _state_score;

			branch->depth = std::max(branch->depth, branch->depth + new_depth);
			branch = branch->parent;
		}

		CombatSequenceNode child_node(_state, _action, _action_size, _state_score, this, _seed);
		child_node.level = this->level + 1;
		child_node.id = _size + 1;
		children.push_back(std::make_shared<CombatSequenceNode>(std::move(child_node)));
	}


	void CombatSequenceTree::addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed) {
		auto packed_state = const_cast<CombatManager&>(manager).packCombatState(_state);

		current->addChild(packed_state, _action, _action_size, _state_score, _seed, size);
		++level_occurence[current->level];
		current = current->children.back().get();
		++size;
		++turns_occurence[_state.turn];
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
			int best_score = 0x0000000000000000;

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