#include "combat_state.h"

#include <iostream>

namespace HotaMechanics {
	CombatState::~CombatState() {
		//std::cout << "del state: " << this << std::endl;
	}

	int64_t CombatState::rehash() {

		int hash = 0;

		if (last_unit != -1) {
			int i = 0;
			int64_t order_hash = 0;
			for (int64_t order_unit : order) {
				order_hash |= (order_unit << (16 * i++));
				if (i == 4) {
					i = 0;
					hash ^= std::hash<int64_t>{}(order_hash);
				}
			}

			hash ^= std::hash<int16_t>{}(~turn);
		}

		hash ^= std::hash<int16_t>{}(~static_cast<int16_t>(result));
		return hash;
	}
}