#include "unit_templates.h"

#include "structures.h"
#include "utils.h"

namespace HotaMechanics {
   using namespace Utils;
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