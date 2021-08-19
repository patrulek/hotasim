#include "mempool.h"

#include <cmath>

#include "../HotaMechanics/combat_state.h"
#include "../HotaMechanics/combat_field.h"
#include "../HotaMechanics/combat_hero.h"
#include "combat_sequencetree.h"

namespace HotaSim {
	using namespace HotaMechanics;

	const int Mempool::BLOCK_SIZE{ 500000 };
	const float Mempool::DEFRAG_FACTOR{ 0.75f };


	std::array<std::shared_ptr<CombatStatePacked>*, 256> Mempool::packed_states;

	std::stack<std::shared_ptr<CombatStatePacked>*> Mempool::freed_state_packed{};

	void Mempool::allocateCombatStatePacked(int _bucket) {
		CombatStatePacked* new_raw_block = new CombatStatePacked[BLOCK_SIZE];
		std::shared_ptr<CombatStatePacked>* new_block = new std::shared_ptr<CombatStatePacked>[BLOCK_SIZE];
		for (int i = 0; i < BLOCK_SIZE; ++i)
			new_block[i].reset(&new_raw_block[i]);
		packed_states[_bucket] = new_block;
		size_state_packed += BLOCK_SIZE;
	}

	std::shared_ptr<CombatStatePacked>& Mempool::retrieveCombatStatePacked(int _size) {
		if (!freed_state_packed.empty() && _size == 1) {
			auto top = freed_state_packed.top();
			freed_state_packed.pop();
			return *top;
		}

		int bucket = (counter_state_packed + _size) / BLOCK_SIZE;
		if (counter_state_packed + _size >= size_state_packed) {
			allocateCombatStatePacked(bucket);
			counter_state_packed = bucket * BLOCK_SIZE + _size;
			return packed_states[bucket][0];
		}

		auto& ret = packed_states[bucket][counter_state_packed % BLOCK_SIZE];
		counter_state_packed += _size;
		return ret;
	}

	void Mempool::freeCombatStatePacked(std::shared_ptr<CombatStatePacked>* _freed) {
		freed_state_packed.push(_freed);
	}

	int Mempool::size_state_packed = 0;
	int Mempool::counter_state_packed = 0;

	// ------------------

	std::array<std::shared_ptr<CombatSequenceNode>*, 256> Mempool::sequence_nodes;

	std::stack<std::shared_ptr<CombatSequenceNode>*> Mempool::freed_sequence_nodes{};

	void Mempool::allocateCombatSequenceNode(int _bucket) {
		CombatSequenceNode* new_raw_block = new CombatSequenceNode[BLOCK_SIZE];
		std::shared_ptr<CombatSequenceNode>* new_block = new std::shared_ptr<CombatSequenceNode>[BLOCK_SIZE];
		for (int i = 0; i < BLOCK_SIZE; ++i)
			new_block[i].reset(&new_raw_block[i]);
		sequence_nodes[_bucket] = new_block;
		size_sequence_nodes += BLOCK_SIZE;
	}

	std::shared_ptr<CombatSequenceNode>& Mempool::retrieveCombatSequenceNode(int _size) {
		if (!freed_sequence_nodes.empty() && _size == 1) {
			auto top = freed_sequence_nodes.top();
			freed_sequence_nodes.pop();
			return *top;
		}

		int bucket = (counter_sequence_nodes + _size) / BLOCK_SIZE;
		if (counter_sequence_nodes + _size >= size_sequence_nodes) {
			allocateCombatSequenceNode(bucket);
			counter_sequence_nodes = bucket * BLOCK_SIZE + _size;
			return sequence_nodes[bucket][0];
		}

		auto& ret = sequence_nodes[bucket][counter_sequence_nodes % BLOCK_SIZE];
		counter_sequence_nodes += _size;
		return ret;
	}

	void Mempool::freeCombatSequenceNode(std::shared_ptr<CombatSequenceNode>* _freed) {
		freed_sequence_nodes.push(_freed);
	}

	int Mempool::size_sequence_nodes = 0;
	int Mempool::counter_sequence_nodes = 0;

	// ------------------


	std::array<CombatUnitPacked*, 512> Mempool::packed_units;

	std::stack<CombatUnitPacked*> Mempool::freed_unit_packed{};

	void Mempool::allocateCombatUnitPacked(int _bucket) {
		try {
			CombatUnitPacked* new_raw_block = new CombatUnitPacked[BLOCK_SIZE];
			packed_units[_bucket] = new_raw_block;
			size_unit_packed += BLOCK_SIZE;
		}
		catch (std::exception& ex) {
			std::cout << "got a problem!\n";
			throw ex;
		}
	}

	CombatUnitPacked* Mempool::retrieveCombatUnitPacked(int _size) {
		//if (!freed_unit_packed.empty() && _size == 1) {
		//	auto top = freed_unit_packed.top();
		//	freed_unit_packed.pop();
		//	return top;
		//}

		int bucket = (counter_unit_packed + _size) / BLOCK_SIZE;
		if (counter_unit_packed + _size >= size_unit_packed) {
			allocateCombatUnitPacked(bucket);
			counter_unit_packed = bucket * BLOCK_SIZE + _size;
			return &packed_units[bucket][0];
		}

		auto& ret = packed_units[bucket][counter_unit_packed % BLOCK_SIZE];
		counter_unit_packed += _size;
		return &ret;
	}

	void Mempool::freeCombatUnitPacked(CombatUnitPacked* _freed) {
		freed_unit_packed.push(_freed);
	}

	int Mempool::size_unit_packed = 0;
	int Mempool::counter_unit_packed = 0;


	// ----------------------------------

	std::array<uint8_t*, 512> Mempool::packed_uint8;

	//std::stack<CombatUnitPacked*> Mempool::freed_unit_packed{};

	void Mempool::allocateUint8(int _bucket) {
		try {
			uint8_t* new_raw_block = new uint8_t[BLOCK_SIZE];
			packed_uint8[_bucket] = new_raw_block;
			size_uint8_packed += BLOCK_SIZE;
		}
		catch (std::exception& ex) {
			std::cout << "got a problem!\n";
			throw ex;
		}
	}

	uint8_t* Mempool::retrieveUint8(int _size) {
		//if (!freed_unit_packed.empty() && _size == 1) {
		//	auto top = freed_unit_packed.top();
		//	freed_unit_packed.pop();
		//	return top;
		//}

		int bucket = (counter_uint8_packed + _size) / BLOCK_SIZE;
		if (counter_uint8_packed + _size >= size_uint8_packed) {
			allocateUint8(bucket);
			counter_uint8_packed = bucket * BLOCK_SIZE + _size;
			return &packed_uint8[bucket][0];
		}

		auto& ret = packed_uint8[bucket][counter_uint8_packed % BLOCK_SIZE];
		counter_uint8_packed += _size;
		return &ret;
	}

	/*void Mempool::freeCombatUnitPacked(CombatUnitPacked* _freed) {
		freed_unit_packed.push(_freed);
	}*/

	int Mempool::size_uint8_packed = 0;
	int Mempool::counter_uint8_packed = 0;

	// -------------------------------------
}