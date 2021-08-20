#pragma once

#include <string>
#include <unordered_map>

#include "structures.h"

namespace HotaMechanics {
	std::unordered_map<std::string, Unit>& unit_templates();

}; // HotaMechanics