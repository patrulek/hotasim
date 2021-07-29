#include "unit_templates.h"

std::unordered_map<std::string, Unit> unit_templates {
/* Simplest units: only walking, no abilities, only melee, etc */
   {"Peasant", { 15, 16, {1, 1, 1, 1, 0, 0, 3, 1, 0}, SpellBook{}, "Peasant" }},
   { "Gremlin", { 55, 16, {3, 3, 1, 2, 0, 0, 4, 4, 0}, SpellBook{}, "Gremlin" } },
   { "Goblin", { 60, 16, {4, 2, 1, 2, 0, 0, 5, 5, 0}, SpellBook{}, "Goblin" } },
   { "Imp", { 50, 16, {2, 3, 1, 2, 0, 0, 3, 4, 0}, SpellBook{}, "Imp" } }
};