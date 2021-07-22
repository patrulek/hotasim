// HotaMechanics.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "combat_manager.h"


 const Unit peasant = {
   15,
   16,
   { 1, 1, 0, 0, 3, 1, 0 },
   SpellBook{}
};

int main()
{

   CombatUnit peasants1000 = CombatUnit(peasant);
   peasants1000.stackNumber = peasants1000.currentStackNumber = 1000;
   peasants1000.hexId = 5 * 17 + 1;
   CombatUnit peasants100 = CombatUnit(peasant);
   peasants100.stackNumber = peasants100.currentStackNumber = 100;
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

   std::cout << "Current unit move: " << unit.currentStackNumber << std::endl << "Unit stats: " << unit.currentStats.atk << " | "
      << unit.currentStats.def << " | " << unit.currentStats.spd << std::endl;


}
