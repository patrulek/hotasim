#include "gtest/gtest.h"

#include <array>
#include "../HotaMechanics/structures.h"

#include "../HotaMechanics/combat_field.h"
#include "utils.h"

TEST(CombatField, shouldReturnCombatHexWithEqualIdAsParam) {
	CombatField field;

	EXPECT_EQ(0, field.getById(0).getId());
	EXPECT_EQ(1, field.getById(1).getId());
	EXPECT_EQ(186, field.getById(186).getId());
	EXPECT_EQ(100, field.getById(100).getId());
	EXPECT_EQ(69, field.getById(69).getId());
}