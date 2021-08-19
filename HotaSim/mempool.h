#pragma once

#include <stack>
#include <array>
#include <memory>

namespace HotaMechanics {
	struct CombatStatePacked;
	struct CombatUnitPacked;
	struct Hero;
	class CombatHero;
	class CombatField;
	struct CombatState;
}

namespace HotaSim {
	struct CombatSequenceNode;
	using namespace HotaMechanics;

	class Mempool {
	public:

		static const int BLOCK_SIZE;
		static const float DEFRAG_FACTOR;

		// packed combat state ----
		static void allocateCombatStatePacked(int _bucket = 0);
		static std::shared_ptr<CombatStatePacked>& retrieveCombatStatePacked(int _size = 1);
		static void freeCombatStatePacked(std::shared_ptr<CombatStatePacked>* _freed);

		static std::array<std::shared_ptr<CombatStatePacked>*, 256> packed_states;
		static std::stack<std::shared_ptr<CombatStatePacked>*> freed_state_packed;
		static int size_state_packed;
		static int counter_state_packed;
		// ----

		// sequence node ----
		static void allocateCombatSequenceNode(int _bucket = 0);
		static std::shared_ptr<CombatSequenceNode>& retrieveCombatSequenceNode(int _size = 1);
		static void freeCombatSequenceNode(std::shared_ptr<CombatSequenceNode>* _freed);

		static std::array<std::shared_ptr<CombatSequenceNode>*, 256> sequence_nodes;
		static std::stack<std::shared_ptr<CombatSequenceNode>*> freed_sequence_nodes;
		static int size_sequence_nodes;
		static int counter_sequence_nodes;
		// ----


		// packed units ----
		static void allocateCombatUnitPacked(int _bucket = 0);
		static CombatUnitPacked* retrieveCombatUnitPacked(int _size = 1);
		static void freeCombatUnitPacked(CombatUnitPacked* _freed);

		static std::array<CombatUnitPacked*, 512> packed_units;
		static std::stack<CombatUnitPacked*> freed_unit_packed;
		static int size_unit_packed;
		static int counter_unit_packed;
		// ----


		// uint8_t ----
		static void allocateUint8(int _bucket = 0);
		static uint8_t* retrieveUint8(int _size = 1);
		//static void freeUint8(CombatUnitPacked* _freed);

		static std::array<uint8_t*, 512> packed_uint8;
		//static std::stack<uint8_t*> freed_unit_packed;
		static int size_uint8_packed;
		static int counter_uint8_packed;

		// ----

	};
}