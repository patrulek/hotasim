#include "gtest/gtest.h"

#include "../HotaMechanics/structures.h"
#include "../HotaMechanics/utils.h"

namespace UtilsTest {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Utils;

	TEST(Utils, shouldConvertIntToStatsCorrectly) {
		BaseStats base_stats = baseStats(2, 3, 5, 0);
		EXPECT_EQ(2, base_stats.atk);
		EXPECT_EQ(3, base_stats.def);
		EXPECT_EQ(5, base_stats.pow);
		EXPECT_EQ(0, base_stats.kgd);

		CombatStats combat_stats = combatStats(1, 3, 7, 0);
		EXPECT_EQ(1, combat_stats.min_dmg);
		EXPECT_EQ(3, combat_stats.max_dmg);
		EXPECT_EQ(7, combat_stats.spd);
		EXPECT_EQ(0, combat_stats.shots);

		PrimaryStats primary_stats = primaryStats(500, 3);
		EXPECT_EQ(500, primary_stats.hp);
		EXPECT_EQ(3, primary_stats.mana);
	}

}; // UtilsTest