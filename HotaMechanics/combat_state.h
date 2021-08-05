#pragma once

#include "combat_hero.h"
#include "combat_field.h"

namespace HotaMechanics {
	struct CombatState {
		explicit CombatState(const CombatHero& _attacker, const CombatHero& _defender, const CombatField& _field)
			: attacker(_attacker), defender(_defender), field(_field) {}
		CombatState() = delete;

		int16_t turn{ -1 };
		int16_t last_unit{ -1 };
		Constants::CombatResult result{ Constants::CombatResult::NOT_STARTED };

		CombatHero attacker;
		CombatHero defender;
		CombatField field;

		std::list<int16_t> order;	// for attacker it will be 0-20; for defender it will be 21-41
	};

}; // HotaMechanics