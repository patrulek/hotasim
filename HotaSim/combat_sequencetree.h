#pragma once

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_state.h"

#include <cstdint>
#include <list>
#include <memory>
#include <random>
#include <functional>
#include <unordered_set>

namespace HotaMechanics {
	class CombatManager;
}


using namespace HotaMechanics;


namespace HotaSim {
	struct StateHash {
		StateHash(CombatState& _state)
			: attacker_hash(_state.attacker.rehash()), defender_hash(_state.defender.rehash()),
			field_hash(_state.field.rehash()), order_hash(_state.rehash()) {}

		bool operator==(const StateHash& _obj) const {
			return attacker_hash == _obj.attacker_hash && defender_hash == _obj.defender_hash
				&& field_hash == _obj.field_hash && order_hash == _obj.order_hash;
		}

		int64_t attacker_hash;
		int64_t defender_hash;
		int64_t field_hash;
		int64_t order_hash;
	};
}

namespace std {
	template<>
	struct std::hash<HotaSim::StateHash> {
		std::size_t operator()(const HotaSim::StateHash& _hash) const noexcept {
			int64_t hash = _hash.attacker_hash;
			hash ^= std::hash<int64_t>{}(_hash.defender_hash);
			hash ^= std::hash<int64_t>{}(_hash.field_hash);
			hash ^= std::hash<int64_t>{}(_hash.order_hash);
			return hash;
		}
	};
}

namespace HotaSim {
	

	struct CombatSequenceNode {
		const std::shared_ptr<CombatStatePacked> state;
		CombatSequenceNode* parent{ nullptr };
		std::vector<std::shared_ptr<CombatSequenceNode>> children;
		int action{ 0 };
		int action_size{ 1 };
		int depth{ 0 };
		int level{ 0 };
		int seed{ 0 };
		int id{ 0 };
		uint64_t score{ 0 };
		uint64_t best_branch_score{ 0 };
		bool cutout{ false };

		CombatSequenceNode(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _score, CombatSequenceNode* _parent, const int _seed)
			: state(_state), action(_action), action_size(_action_size), score(_score), best_branch_score(_score), parent(_parent), seed(_seed) {

			children.reserve(64);
		}

		void addChild(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const int _size);
	};

	class CombatSequenceTree
	{
	public:
		const CombatManager& manager;
		std::shared_ptr<CombatSequenceNode> root{ nullptr };
		CombatSequenceNode* current{ nullptr };
		uint64_t size{ 0 };

		int fp_cnt{ 0 };
		int since_last_sort{ 0 };
		std::vector<CombatSequenceNode*> forgotten_paths;
		std::vector<CombatSequenceNode*> circular_paths;
		//std::unordered_set<StateHash> state_hashes;
		std::unordered_map<StateHash, CombatState*> state_hashes;
		std::unordered_map<StateHash, CombatSequenceNode*> node_hashes;
		int circular_occurence{ 0 };
		std::array<int, 32> turns_occurence;
		std::array<int, 128> level_occurence;

		CombatSequenceTree(const HotaMechanics::CombatManager& _manager, const CombatState& _initial_state, const uint64_t _initial_state_score = 0x0000800080008000);

		bool hasParent() {
			return current->parent != root.get();
		}
		bool unitStackLastAction() {
			return current->action + 1 == current->action_size;
		}
		void addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed);
		bool isCurrentRoot() const { return current == root.get(); }

		void sortForgotten();
		void takeForgotten();

		int getSize() const { return size; }
		const bool canTakeForgotten() const { return fp_cnt >= forgotten_paths.size(); }

		void goParent();
		void goRandomParent(const bool _combat_finished);
		void goRoot(const bool _combat_finished);

		bool foundCircularPath() { 
			bool result = circular_path_found;
			circular_path_found = false;
			return result; 
		}
		CombatSequenceNode* findBestLeaf();

		bool circular_path_found{ false };
	};

}; // HotaSim