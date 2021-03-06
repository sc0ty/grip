#include "catch2/catch.hpp"

#include "ids.h"

using namespace std;


TEST_CASE("CompressedIds composition", "[CompressedIds]")
{
	SECTION("Adding Ids", "[CompressedIds]")
	{
		CompressedIds ids1;

		ids1.add( 0 );
		REQUIRE( CMP_IDS(ids1, 0) );

		ids1.add( 1 );
		REQUIRE( CMP_IDS(ids1, 0, 1) );

		ids1.add( 1234 );
		REQUIRE( CMP_IDS(ids1, 0, 1, 1234) );

		ids1.add( (uint32_t) -2 );
		REQUIRE( CMP_IDS(ids1, 0, 1, 1234, (uint32_t)-2) );

		ids1.add( (uint32_t) -1 );
		REQUIRE( CMP_IDS(ids1, 0, 1, 1234, (uint32_t)-2, (uint32_t)-1) );

		REQUIRE_THROWS_AS( ids1.add( 5 ), Error );
		REQUIRE( CMP_IDS(ids1, 0, 1, 1234, (uint32_t)-2, (uint32_t)-1) );

		REQUIRE_THROWS_AS( ids1.add( 1234 ), Error );
		REQUIRE( CMP_IDS(ids1, 0, 1, 1234, (uint32_t)-2, (uint32_t)-1) );

		ids1.clear();

		ids1.add( 10 );
		ids1.add( 20 );
		REQUIRE( CMP_IDS(ids1, 10, 20) );
	}

	SECTION("Adding long sequence of Ids", "[CompressedIds]")
	{
		CompressedIds ids1;
		vector<uint32_t> vec;

		for (uint32_t id = 123; id < 1234; id++)
		{
			ids1.add(id);
			vec.push_back(id);
		}
		REQUIRE( compareIds(ids1, vec) );

		for (uint32_t id = 2000; id < 3000; id += 0x3f)
		{
			ids1.add(id);
			vec.push_back(id);
		}
		REQUIRE( compareIds(ids1, vec) );

		for (uint32_t id = 4000; id < 5000; id += 0x7f)
		{
			ids1.add(id);
			vec.push_back(id);
		}
		REQUIRE( compareIds(ids1, vec) );
	}
}

TEST_CASE("CompressedIds modification", "[CompressedIds]")
{
	SECTION("Swapping Ids", "[CompressedIds]")
	{
		CompressedIds ids1;
		ids1.add(4);
		ids1.add(5);
		ids1.add(8);
		ids1.add(20);
		REQUIRE( CMP_IDS(ids1, 4, 5, 8, 20) );

		CompressedIds ids2;
		ids2.add(2);
		ids2.add(15);
		REQUIRE( CMP_IDS(ids2, 2, 15) );

		ids1.swap(ids2);
		REQUIRE( CMP_IDS(ids1, 2, 15) );
		REQUIRE( CMP_IDS(ids2, 4, 5, 8, 20) );

		ids2.clear();
		ids1.swap(ids2);
		REQUIRE( ids1.empty() );
		REQUIRE( CMP_IDS(ids2, 2, 15) );

		ids2.clear();
		ids2.swap(ids1);
		REQUIRE( ids1.empty() );
		REQUIRE( ids2.empty() );
	}

	SECTION("Moving Ids", "[CompressedIds]")
	{
		CompressedIds ids1;
		ids1.add( 100 );
		ids1.add( 200 );
		ids1.add( 201 );

		CompressedIds ids2;
		ids2.add( 5 );

		ids2.move(ids1);
		REQUIRE( ids1.empty() );
		REQUIRE( CMP_IDS(ids2, 100, 200, 201) );

		ids2.move(ids1);
		REQUIRE( ids1.empty() );
		REQUIRE( ids2.empty() );
	}
}

TEST_CASE("Ids compression", "[CompressedIds]")
{
	SECTION("Decompress IDS", "[CompressedIds]")
	{
		vector<uint32_t> vec;
		CompressedIds cids;
		Ids ids;

		cids.decompress(ids);
		REQUIRE( ids.empty() );

		for (uint32_t id = 123; id < 1234; id++)
		{
			cids.add(id);
			vec.push_back(id);
		}
		cids.decompress( ids );
		REQUIRE( compareIds(ids, vec) );

		for (uint32_t id = 2000; id < 3000; id += 0x3f)
		{
			cids.add(id);
			vec.push_back(id);
		}
		ids.clear();
		cids.decompress( ids );
		REQUIRE( compareIds(ids, vec) );

		for (uint32_t id = 4000; id < 5000; id += 0x7f)
		{
			cids.add(id);
			vec.push_back(id);
		}
		ids.clear();
		cids.decompress( ids );
		REQUIRE( compareIds(ids, vec) );
	}
}
