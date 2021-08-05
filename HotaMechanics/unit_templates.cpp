#include "unit_templates.h"

#include "structures.h"

namespace HotaMechanics {

   BaseStats baseStats(int _atk, int _def, int _pow, int _kgd) {
      return BaseStats{ _atk << 24 | _def << 16 | _pow << 8 | _kgd };
   }

   CombatStats combatStats(int _min, int _max, int _spd, int _shots) {
      return CombatStats{ _min << 24 | _max << 16 | _spd << 8 | _shots };
   }

   PrimaryStats primaryStats(int _hp, int _mana) {
      return PrimaryStats{ _hp << 16 | _mana };
   }

   /* Simplest units: only walking, no abilities, only melee, etc */
   std::unordered_map<std::string, Unit> unit_templates() {
      static std::unordered_map<std::string, Unit> tmp;
      if (!tmp.empty())
         return tmp;

      tmp["Peasant"] = Unit{ {baseStats(1, 1, 0, 0), combatStats(1, 1, 3, 0), primaryStats(1, 0), 15}, "Peasant" };
      tmp["Gremlin"] = Unit{ {baseStats(3, 3, 0, 0), combatStats(1, 2, 4, 0), primaryStats(4, 0), 55}, "Gremlin" };
      tmp["Goblin"] = Unit{ {baseStats(4, 2, 0, 0), combatStats(1, 2, 5, 0), primaryStats(5, 0), 60}, "Goblin" };
      tmp["Imp"] = Unit{ {baseStats(2, 3, 0, 0), combatStats(1, 2, 5, 0), primaryStats(4, 0), 50}, "Imp" };

      return tmp;
   }

}; // HotaMechanics