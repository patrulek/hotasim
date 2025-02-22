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
	struct CombatSerializer;

	struct CombatSequenceNode {
		std::shared_ptr<CombatStatePacked> state;
		CombatSequenceNode* parent{ nullptr };
		std::vector<std::shared_ptr<CombatSequenceNode>> children;
		int action{ 0 };
		int action_size{ 1 };
		int depth{ 0 };
		int level{ 0 };
		int seed{ 0 };
		int turn{ -1 };
		uint64_t id{ 0 };
		uint64_t score{ 0 };
		uint64_t best_branch_score{ 0 };
		bool cutout{ false };

		void addChild(std::shared_ptr<CombatStatePacked> _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const uint64_t _size, const bool _player_won);
	};

	class CombatSequenceTree
	{
	public:
		const CombatSerializer& serializer;
		const CombatManager& manager;
		std::shared_ptr<CombatSequenceNode> root{ nullptr };
		CombatSequenceNode* current{ nullptr };
		uint64_t size{ 0 };
		CombatSequenceNode* best_leaf{ nullptr };

		bool circular_path_found{ false };
		int since_last_sort{ 0 };
		std::vector<CombatSequenceNode*> circular_paths;
		//std::unordered_set<StateHash> state_hashes;
		std::unordered_map<StateHash, int16_t> state_hashes;
		std::unordered_map<StateHash, CombatSequenceNode*> node_hashes;
		int circular_occurence{ 0 };
		int early_cutoff{ 0 };
		uint64_t best_score{ 0x0000800000000000 };
		int best_turns{ -1 };
		bool need_sort{ false };
		std::array<int, 32> turns_occurence;
		std::array<int, 128> level_occurence;
		std::unordered_map<uint64_t, std::vector<std::shared_ptr<CombatSequenceNode>*>> nodes_to_release;

		std::vector<std::shared_ptr<CombatSequenceNode>*> node_order;
		std::unordered_set<std::shared_ptr<CombatSequenceNode>*> taken;

		CombatSequenceTree(const CombatSerializer& _serializer, const HotaMechanics::CombatManager& _manager, 
			const CombatState& _initial_state, const uint64_t _initial_state_score = 0x0000800000000000);

		bool hasParent() {
			return current->parent != root.get();
		}
		bool unitStackLastAction() {
			return current->action + 1 == current->action_size;
		}
		void addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed, const bool _first_ai_attack = false);
		bool isCurrentRoot() const { return current == root.get(); }

		void takeBest(const bool _halving = false);

		uint64_t getSize() const { return size; }

		void goParent();

		bool foundCircularPath() { 
			bool result = circular_path_found;
			circular_path_found = false;
			return result; 
		}
		CombatSequenceNode* findBestLeaf();

	};

}; // HotaSim