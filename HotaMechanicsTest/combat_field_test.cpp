#include "gtest/gtest.h"

#include "utils.h"
#include "../HotaMechanics/utils.h"
#include "../HotaMechanics/combat_field.h"

namespace CombatFieldTest {
	using namespace HotaMechanics;
	using namespace HotaMechanics::Utils;
	using namespace TestUtils;

	// CombatField::isHexWalkable()
	TEST(CombatField, shouldReturnTrueIfHexIsWalkableFalseOtherwise) {
		CombatField field(createField());

		EXPECT_FALSE(field.isHexWalkable(INVALID_HEX_ID)); // -1 is "no found" hex, so it cant be walkable

		field.fillHex(1, CombatHexOccupation::UNIT); // hex occupied by unit is not walkable
		EXPECT_FALSE(field.isHexWalkable(1));

		field.fillHex(2, CombatHexOccupation::SOFT_OBSTACLE); // hex occupied by soft obstacle (firewall, quicksand, etc, casted but not removable) is walkable
		EXPECT_TRUE(field.isHexWalkable(2));

		field.fillHex(3, CombatHexOccupation::SOLID_OBSTACLE); // hex occupied by solid obstacle (the one generated by game, but removable) is not walkable
		EXPECT_FALSE(field.isHexWalkable(3));

		field.fillHex(4, CombatHexOccupation::INDESTRUCTIBLE_OBSTACLE); // hex occupied by indestructible obstacle (force field/ the one generated by game) is not walkable
		EXPECT_FALSE(field.isHexWalkable(4));

		EXPECT_TRUE(field.isHexWalkable(5)); // empty hex is walkable

		EXPECT_FALSE(field.isHexWalkable(0)); // hex in first column of field is not walkable
		EXPECT_FALSE(field.isHexWalkable(34));

		EXPECT_FALSE(field.isHexWalkable(16)); // hex in last column of field is not walkable
		EXPECT_FALSE(field.isHexWalkable(50));
	}

	// CombatField::fillHex(id, occupation), CombatField::clearHex(id)
	TEST(CombatField, shouldClearAndFillHexSetCorrectOccupation) {
		CombatField field(createField());

		EXPECT_EQ(CombatHexOccupation::EMPTY, field.getById(32).getOccupation());
		field.fillHex(32, CombatHexOccupation::INDESTRUCTIBLE_OBSTACLE);
		EXPECT_EQ(CombatHexOccupation::INDESTRUCTIBLE_OBSTACLE, field.getById(32).getOccupation());
		EXPECT_TRUE(field.getOccupied()[32]);

		field.clearHex(32);
		EXPECT_EQ(CombatHexOccupation::EMPTY, field.getById(32).getOccupation());
		EXPECT_FALSE(field.getOccupied()[32]);
	}

	// CombatField::setTemplate(vector<int>)
	TEST(CombatField, shouldSetTemplateSetCorrectHexesOccupation) {
		CombatField field(createField());

		int empty_values = 0;
		for (HexId hex : range(FIELD_SIZE))
			empty_values += field.getById(hex).getOccupation() == CombatHexOccupation::EMPTY;
		EXPECT_EQ(FIELD_SIZE, empty_values); // new field without template should have all empty hexes

		field = CombatField(createField(CombatFieldType::GRASS, CombatFieldTemplate::IMPS_2x100));

		empty_values = 0;
		for (HexId hex : range(FIELD_SIZE))
			empty_values += field.getById(hex).getOccupation() == CombatHexOccupation::EMPTY;
		EXPECT_NE(FIELD_SIZE, empty_values); // setting custom template with obstacles should set some

		empty_values = 0;
		field.setTemplate(CombatFieldTemplate::EMPTY);
		for (HexId hex : range(FIELD_SIZE))
			empty_values += field.getById(hex).getOccupation() == CombatHexOccupation::EMPTY;
		EXPECT_EQ(FIELD_SIZE, empty_values); // were changing field template back to empty
	}

	TEST(CombatField, shouldReturnEqualHashesForSameFields) {
		CombatField field(createField());
		CombatField field2(createField());

		Hash h1 = field.rehash();
		Hash h2 = field2.rehash();

		EXPECT_EQ(h1, h2);

		field2.fillHex(32, CombatHexOccupation::UNIT);
		h2 = field2.rehash();
		
		EXPECT_NE(h1, h2);

		field2.clearHex(32);
		h2 = field2.rehash();

		EXPECT_EQ(h1, h2);
	}

	TEST(CombatField, shouldReturnDifferentHashesForDifferentFields) {
		CombatField field(createField());
		CombatField field2(createField());
		field2.fillHex(32, CombatHexOccupation::UNIT);

		Hash h1 = field.rehash();
		Hash h2 = field2.rehash();

		EXPECT_NE(h1, h2);

		field2.clearHex(32);
		field2.fillHex(33, CombatHexOccupation::UNIT);
		h2 = field2.rehash();

		EXPECT_NE(h1, h2);

		CombatField field3(createField(CombatFieldType::GRASS, CombatFieldTemplate::IMPS_2x100));
		Hash h3 = field3.rehash();

		EXPECT_NE(h1, h3);
		EXPECT_NE(h2, h3);
	}

	TEST(CombatField, shouldReturnDifferentHashesForDifferentCombinations) {
		CombatField field(createField());
		field.fillHex(3, CombatHexOccupation::UNIT);
		field.fillHex(37, CombatHexOccupation::UNIT);

		CombatField field2(createField());
		field2.fillHex(32, CombatHexOccupation::UNIT);
		field2.fillHex(35, CombatHexOccupation::UNIT);
		field2.fillHex(37, CombatHexOccupation::UNIT);

		Hash h1 = field.rehash();
		Hash h2 = field2.rehash();

		EXPECT_NE(h1, h2);
	}

	TEST(CombatField, shouldGenerateUniqueHashes) {
		std::unordered_map<Hash, HexSet> hashes;
		std::unordered_map<Hash, std::pair<int, HexSet>> search_hashes;

		for (int i = 0; i < 100000; ++i) {
			CombatField field(createField());
			int unit_fields = getRandomInt(2, 20);
			int obstacle_fields = getRandomInt(0, 10);

			for (int j = 0; j < unit_fields; ++j) {
				int hex = getRandomInt(0, FIELD_SIZE - 1);
				while (field.getOccupied()[hex])
					hex = getRandomInt(0, FIELD_SIZE - 1);
				field.fillHex(hex, CombatHexOccupation::UNIT);
			}

			for (int j = 0; j < obstacle_fields; ++j) {
				int hex = getRandomInt(0, FIELD_SIZE - 1);
				while (field.getOccupied()[hex])
					hex = getRandomInt(0, FIELD_SIZE - 1);
				field.fillHex(hex, CombatHexOccupation::SOLID_OBSTACLE);
			}

			field.rehash();
			if (hashes.find(field.getHash()) != std::end(hashes)) {
				EXPECT_TRUE(hashes[field.getHash()] == field.getOccupied());
			}
			hashes[field.getHash()] = field.getOccupied();

			HexId source_hex = getRandomInt(1, FIELD_SIZE - 1);
			Hash h = field.getHash();
			h ^= std::hash<Hash>{}(static_cast<Hash>(source_hex) << 8);

			if (search_hashes.find(h) != std::end(search_hashes)) {
				EXPECT_EQ(source_hex, search_hashes[h].first);
				EXPECT_EQ(field.getOccupied(), search_hashes[h].second);
			}
			search_hashes[h] = std::make_pair(source_hex, field.getOccupied());
		}
	}
};