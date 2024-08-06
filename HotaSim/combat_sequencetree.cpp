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
		state_hashes.rehash(262144);
		node_order.reserve(262144);
		turns_occurence.fill(0);
		level_occurence.fill(0);
	}



	void CombatSequenceNode::addChild(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const uint64_t _size, const bool _player_won) {
		CombatSequenceNode* branch = this;
		int new_depth = children.empty();

		while (branch) {
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
		child_node->turn = _state->turn;
		if (child_node->action_size > 1)
			children.reserve(child_node->action_size);

		if (child_node->turn > this->turn || _player_won) {
			branch = this;
			while (branch) {
				if (_state_score > branch->best_branch_score)
					branch->best_branch_score = _state_score;
				branch = branch->parent;
			}
		}

		children.emplace_back(std::move(child_node));


	}


	void CombatSequenceTree::addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const bool _first_ai_attack) {
		circular_path_found = false;

		//if (best_score & 0xFFFF000000000000) {
		//	if ((_state_score & 0xFFFF000000000000) <= (best_score & 0xFFFF000000000000) &&
		//		(_state_score & 0x0000FFFFFFFFFFFF) < (best_score & 0x0000FFFFFFFFFFFF) && _state.turn >= best_turns) {
		//		++early_cutoff;
		//		circular_path_found = true;
		//		return;
		//	}
		//}

		//if (_first_ai_attack) {
		//	++early_cutoff;
		//	circular_path_found = true;
		//	return;
		//}


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

		++level_occurence[current->level];
		++size;
		++turns_occurence[_state.turn];
		auto packed_state = const_cast<CombatSerializer&>(serializer).packCombatState(_state);
		auto bbs = root->best_branch_score;
		current->addChild(packed_state, _action, _action_size, _state_score, _seed, size, manager.isCombatFinished());

		//node_hashes[StateHash(const_cast<CombatState&>(_state))] = current->children.back().get();
		//if (_state_score > 0x0000800000008000) {
		//	nodes_to_release[_state_score].push_back(&current->children.back());
		//}
		if (current->level > 0 && _action_size > 1 && taken.find(&current->children.back()) == std::end(taken)) {
			need_sort = true;
			node_order.push_back(&current->children.back());
		}
		current = current->children.back().get();

		if (bbs < current->best_branch_score) {
			best_leaf = current;
		}

		if (_state_score > best_score ) {
			best_score = _state_score;
			best_turns = _state.turn;

			//for (auto& ntr : nodes_to_release) {
			//	for (auto& node : ntr.second) {
			//		if ((*node)->best_branch_score < best_score) {
			//			Mempool::freeCombatStatePacked(&(*node)->state);
			//			Mempool::freeCombatSequenceNode(&(*node));
			//		}
			//	}
			//}

			//nodes_to_release.clear();
		}
	}

	void CombatSequenceTree::takeBest(const bool _halving) {
		
		if (need_sort) {
			std::sort(std::begin(node_order), std::end(node_order), [](auto& n1, auto& n2) {
				return (0xFFFFFFFF00FFFFFF & (*n1)->score) < (0xFFFFFFFF00FFFFFF & (*n2)->score);
				});
			need_sort = false;
		}

		current = node_order.back()->get();
		taken.insert(node_order.back());
		node_order.pop_back();

		std::vector<std::shared_ptr<CombatSequenceNode>*> node_order2; node_order2.reserve(5000);


		if (_halving && node_order.size() > 1000) {
			//node_order.erase(std::begin(node_order) + 450, std::begin(node_order) + node_order.size() - 450);
			std::array<int, 16> occurences{}; occurences.fill(0);

			for (auto no : node_order) {
				occurences[(*no)->turn]++;
			}

			int m = 0;
			for (auto occ : occurences)
				m = std::max(m, occ);

			occurences.fill(0);

			for (auto no : node_order) {
				//if (occurences[(*no)->turn]++ < m / 1024)
				if((*no)->turn <= 3)
					node_order2.push_back(no);
			}

			node_order2.erase(std::begin(node_order2) + 150, std::begin(node_order2) + node_order2.size() - 50);
			node_order = node_order2;
		}
		circular_path_found = false;

		if (current->score > best_leaf->score) {
			best_leaf = current;
		}
	}

	

	void CombatSequenceTree::goParent() {
		if (!current->parent)
			throw std::exception("Should never happen (cant go to null parent node)");

		current = current->parent;
	}

	

	CombatSequenceNode* CombatSequenceTree::findBestLeaf() {
		return best_leaf;
		/*CombatSequenceNode* branch = root.get();

		while (!branch->children.empty()) {
			int best_depth = 999;
			int best_idx = 0, idx = 0;
			uint64_t best_score = 0x0000800000000000;

			if (branch->children.size() > 1) {
				for (auto& child : branch->children) {
					if ((child->best_branch_score == branch->best_branch_score )) {
						best_idx = idx;
						best_score = child->score;
					}
					++idx;
				}
			}

			branch = branch->children[best_idx].get();
		}

		return branch;*/
	}
}