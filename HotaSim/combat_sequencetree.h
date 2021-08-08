#pragma once

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_state.h"

#include <cstdint>
#include <list>
#include <memory>
#include <random>

using namespace HotaMechanics;

namespace HotaSim {

	struct CombatSequenceNode {
		const CombatState state;
		CombatSequenceNode* parent{ nullptr };
		std::list<std::shared_ptr<CombatSequenceNode>> children;
		int action{ 0 };
		int action_size{ 1 };
		int depth{ 0 };
		int seed{ 0 };
		uint64_t score{ 0 };
		uint64_t best_branch_score{ 0 };
		bool cutout{ false };

		CombatSequenceNode(const CombatState& _state, const int _action, const int _action_size, const uint64_t _score, CombatSequenceNode* _parent, const int _seed)
			: state(_state), action(_action), action_size(_action_size), seed(_seed), score(_score), best_branch_score(_score), parent(_parent) {
		}


		void addChild(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed) {
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
	};

	class CombatSequenceTree
	{
	public:
		std::shared_ptr<CombatSequenceNode> root{ nullptr };
		CombatSequenceNode* current{ nullptr };
		uint64_t size{ 0 };

		int fp_cnt{ 0 };
		int since_last_sort{ 0 };
		std::vector<CombatSequenceNode*> forgotten_paths;
		std::array<int, 32> turns_occurence;

		CombatSequenceTree(const CombatState& _initial_state, const uint64_t _initial_state_score = 0x0000800080008000) {
			root = std::make_shared<CombatSequenceNode>(_initial_state, 0, 1, _initial_state_score, nullptr, 1);
			current = root.get();
			forgotten_paths.reserve(8096);
			turns_occurence.fill(0);
		}

		void addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed) {
			current->addChild(_state, _action, _action_size, _state_score, _seed);
			current = current->children.back().get();
			++size;
			++turns_occurence[_state.turn];
		}

		bool isCurrentRoot() const {
			return current == root.get();
		}

		void sortForgotten() {
			std::sort(std::begin(forgotten_paths) + fp_cnt, std::end(forgotten_paths), [](auto _node1, auto _node2) { return _node1->best_branch_score > _node2->best_branch_score; });
			since_last_sort = 0;
		}

		void takeForgotten() {
			if (since_last_sort > 500)
				sortForgotten();

			current = forgotten_paths[fp_cnt++];
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

		void goRandomParent() {
			if (current->depth < 6) {
				goRoot();
				return;
			}


			forgotten_paths.push_back(current);
			while (current->parent && current->parent->parent != root.get() && current->state.turn > 1)
				goParent();
		}

		void goRoot() {
			forgotten_paths.push_back(current);
			while (current->parent && current->parent->parent != root.get())
				goParent();
		}

		CombatSequenceNode* findBestLeaf() {
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
	};

}; // HotaSim