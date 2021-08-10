#include "combat_sequencetree.h"


namespace HotaSim {

	void CombatSequenceNode::addChild(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed) {
		score = _state_score;

		CombatSequenceNode* branch = this;
		int new_depth = children.empty();

		while (branch) {
			if (_state_score > branch->best_branch_score)
				branch->best_branch_score = _state_score;

			branch->depth = std::max(branch->depth, new_depth++);
			branch = branch->parent;
		}

		auto child = std::make_shared<CombatSequenceNode>(_state, _action, _action_size, _state_score, this, _seed);
		children.push_back(child);
	}


	void CombatSequenceTree::addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed) {
		current->addChild(_state, _action, _action_size, _state_score, _seed);
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
		if (current->depth < 6) {
			goRoot(_combat_finished);
			return;
		}

		if (!_combat_finished)
			forgotten_paths.push_back(current);

		while (current->parent && current->parent->parent != root.get() && current->state.turn > 1)
			goParent();
	}

	void CombatSequenceTree::goRoot(const bool _combat_finished) {
		if (!_combat_finished)
			forgotten_paths.push_back(current);

		while (current->parent && current->parent->parent != root.get())
			goParent();
	}

	CombatSequenceNode* CombatSequenceTree::findBestLeaf() {
		CombatSequenceNode* branch = root.get();

		while (!branch->children.empty()) {
			int best_depth = 999;
			int best_idx = 0, idx = 0;

			for (auto child : branch->children) {
				if (child->best_branch_score == branch->best_branch_score && child->depth < best_depth) {
					best_idx = idx;
				}
				++idx;
			}

			auto it = std::begin(branch->children);
			std::advance(it, best_idx);
			branch = (*it).get();
		}

		return branch;
	}
}