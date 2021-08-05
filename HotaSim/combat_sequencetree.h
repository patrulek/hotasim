#pragma once

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_state.h"

#include <cstdint>
#include <list>
#include <memory>

using namespace HotaMechanics;

struct CombatSequenceNode {
	const CombatState state;
	std::shared_ptr<CombatSequenceNode> parent{ nullptr };
	std::list<std::shared_ptr<CombatSequenceNode>> children;
	int action{ 0 };
	int action_size{ 1 };
	int64_t score{ 0 };
	int64_t best_branch_score{ 0 };
	bool cutout{ false };

	CombatSequenceNode(const CombatState& _state, const int64_t _score)
		: CombatSequenceNode(_state, _score, nullptr) {}

	CombatSequenceNode(const CombatState& _state, const int _action, const int _action_size, const int64_t _score, CombatSequenceNode* _parent)
		: state(_state), action(_action), action_size(_action_size), score(_score), best_branch_score(_score) {
		parent.reset(_parent);
	}

	CombatSequenceNode(const CombatState& _state, const int64_t _score, std::shared_ptr<CombatSequenceNode> _parent)
		: state(_state), score(_score), best_branch_score(_score), parent(_parent) {}

	CombatSequenceNode(const CombatState& _state, const int _action, const int _action_size, const int64_t _score, std::shared_ptr<CombatSequenceNode> _parent)
		: state(_state), action(_action), action_size(_action_size), score(_score), best_branch_score(_score), parent(_parent) {}

	void addChild(const CombatState& _state, const int _action, const int _action_size, const int64_t _state_score) {
		score = _state_score;

		CombatSequenceNode* branch = this;
		while (branch) {
			if (_state_score <= branch->best_branch_score)
				break;

			branch->best_branch_score = _state_score;
			branch = branch->parent.get();
		}

		auto child = std::make_shared<CombatSequenceNode>(_state, _action, _action_size, _state_score, this);
		children.push_back(child);
	}
};

class CombatSequenceTree
{
public:
	std::shared_ptr<CombatSequenceNode> root{ nullptr };
	std::shared_ptr<CombatSequenceNode> current{ nullptr };
	int64_t size{ 0 };

	CombatSequenceTree(const CombatState& _initial_state, const int64_t _initial_state_score = 0x0000800080008000) {
		root = std::make_shared<CombatSequenceNode>(_initial_state, _initial_state_score);
		current = root;
	}

	void addState(const CombatState& _state, const int _action, const int _action_size, const int64_t _state_score) {
		current->addChild(_state, _action, _action_size, _state_score);
		current = current->children.back();
		++size;
	}

	bool isCurrentRoot() const {
		return current.get() == root.get();
	}

	int branchSize(CombatSequenceNode* _branch) {
		if (_branch->children.empty())
			return 1;

		int sum = 1;
		for (auto child : _branch->children)
			sum += branchSize(child.get());

		return sum;
	}

	int getSize() const {
		return size;
	}

	int calcSize() {
		CombatSequenceNode* main_branch = root.get();
		return branchSize(main_branch);
	}

	void goParent() {
		if (!current->parent)
			throw std::exception("Should never happen (cant go to null parent node)");

		current = current->parent;
	}

	CombatSequenceNode* findBestLeaf() {
		CombatSequenceNode* branch = root.get();

		while (!branch->children.empty()) {
			for (auto child : branch->children) {
				if (child->score == branch->best_branch_score) {
					branch = child.get();
					break;
				}
			}
		}

		return branch;
	}
};

