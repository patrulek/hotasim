// HotaMechanics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/combat_manager.h"
#include "../HotaMechanics/unit_templates.h"

#include "utils.h"
#include "combat_simulator.h"

using namespace HotaMechanics;
using namespace HotaMechanics::Constants;
using namespace HotaSim::Utils;
using namespace HotaSim;

int main()
{
   // prepare/load battle template
   HeroCreator attacker;
   attacker.setAttack(0); attacker.setDefense(0);
   attacker.addUnit(unit_templates()["Peasant"], 1);
   attacker.addUnit(unit_templates()["Peasant"], 1);
   /*attacker.addUnit(unit_templates()["Imp"], 50);
   attacker.addUnit(unit_templates()["Imp"], 100);*/
   //attacker.addUnit(unit_templates()["Imp"], 50);
   //attacker.addUnit(unit_templates()["Imp"], 96);
   //attacker.addUnit(unit_templates()["Imp"], 50);
   //attacker.addUnit(unit_templates()["Imp"], 1);
   //attacker.addUnit(unit_templates()["Imp"], 1);
   //attacker.addUnit(unit_templates()["Imp"], 1);
   //attacker.addUnit(unit_templates()["Imp"], 1);

   HeroCreator defender;
   defender.setAttack(0), defender.setDefense(0);
   defender.addUnit(unit_templates()["Peasant"], 1);
   //defender.addUnit(unit_templates()["Peasant"], 250);
   //defender.addUnit(unit_templates()["Peasant"], 250);

   CombatFieldType field_type = CombatFieldType::GRASS;
   CombatType combat_type = CombatType::NEUTRAL;
   // ---------------------

   // create combat simulator
   CombatSimulator hotaSim(attacker.hero, defender.hero, field_type, combat_type);
   hotaSim.initialize();
   hotaSim.start();

   std::cout << "\n\nPress any key...\n";
   getchar();
   // ---------------------

   /*
   CombatUnit peasants1000 = CombatUnit(unit_templates["Peasant"]);
   peasants1000.stackNumber = peasants1000.stackNumber = 1000;
   peasants1000.hexId = 5 * 17 + 1;
   CombatUnit peasants100 = CombatUnit(unit_templates["Peasant"]);
   peasants100.stackNumber = peasants100.stackNumber = 100;
   peasants100.hexId = 5 * 17 + 14;

   CombatHero player = CombatHero();
   player.stats.atk = player.stats.def = 2;
   player.units[0] = peasants1000;
   CombatHero enemy = CombatHero();
   enemy.stats.atk = enemy.stats.def = 0;
   enemy.units[0] = peasants100;

   CombatField field = CombatField();

   CombatManager mgr(player, enemy, field);

   auto initState = mgr.duplicateCurrentState();
   mgr.nextState();
   auto state = mgr.getCurrentState();

   std::cout << int(initState.result) << std::endl << int(state.result) << std::endl;

   state.turn = 0;
   state.unitOrder[0] = 21; state.unitOrder[1] = 0;
   state.currentUnit = 0;

   auto unit = mgr.getActiveStack();

   std::cout << "Current unit move: " << unit.stackNumber << std::endl << "Unit stats: " << unit.currentStats.atk << " | "
      << unit.currentStats.def << " | " << unit.currentStats.spd << std::endl;
   */
}
