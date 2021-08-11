#pragma once

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_state.h"

#include <cstdint>
#include <list>
#include <memory>
#include <random>

namespace HotaMechanics {
	class CombatManager;
}


using namespace HotaMechanics;

namespace HotaSim {

	struct CombatSequenceNode {
		const std::shared_ptr<CombatStatePacked> state;
		CombatSequenceNode* parent{ nullptr };
		std::list<std::shared_ptr<CombatSequenceNode>> children;
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
		std::vector<CombatSequenceNode*> deadend_paths;
		std::array<int, 32> turns_occurence;
		std::array<int, 32> level_occurence;

		CombatSequenceTree(const HotaMechanics::CombatManager& _manager, const CombatState& _initial_state, const uint64_t _initial_state_score = 0x0000800080008000);

		void addState(const CombatState& _state, const int _action, const int _action_size, const uint64_t _state_score, const int _seed);
		bool isCurrentRoot() const { return current == root.get(); }

		void sortForgotten();
		void takeForgotten();

		int getSize() const { return size; }
		const bool canTakeForgotten() const { return fp_cnt >= forgotten_paths.size(); }

		void goParent();
		void goRandomParent(const bool _combat_finished);
		void goRoot(const bool _combat_finished);

		CombatSequenceNode* findBestLeaf();
	};

}; // HotaSim