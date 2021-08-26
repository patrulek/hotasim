#include "combat_state.h"

#include <iostream>

namespace HotaMechanics {
	CombatState::~CombatState() {
		//std::cout << "del state: " << this << std::endl;
	}

	Hash CombatState::rehash() {

		int hash = 0;

		if (last_unit != -1) {
			int i = 0;
			Hash order_hash = 0;
			for (Hash order_unit : order) {
				order_hash |= (order_unit << (16 * i++));
				if (i == 4) {
					i = 0;
					hash ^= std::hash<Hash>{}(order_hash);
				}
			}

			hash ^= std::hash<Hash>{}(~turn);
		}

		hash ^= std::hash<Hash>{}(~static_cast<Hash>(result));
		return hash;
	}
}