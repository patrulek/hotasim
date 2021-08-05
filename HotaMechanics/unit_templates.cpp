#include "unit_templates.h"

#include "structures.h"

namespace HotaMechanics {

   BaseStats baseStats(int _atk, int _def, int _pow, int _kgd) {
      return BaseStats{ _kgd << 24 | _pow << 16 | _def << 8 | _atk };
   }

   CombatStats combatStats(int _min, int _max, int _spd, int _shots) {
      return CombatStats{ _shots << 24 | _spd << 16 | _max << 8 | _min };
   }

   PrimaryStats primaryStats(int _hp, int _mana) {
      return PrimaryStats{ _mana << 16 | _hp };
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