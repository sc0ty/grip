#include "catch.hpp"
#include "../grip/pattern.h"
#include <map>

using namespace std;


static bool patternMatch(const Pattern *pattern, const char *str, size_t pos, size_t len)
{
	Pattern::Match m = pattern->match(str);
	return (m.pos == str + pos) && (m.len == len);
}

static bool patternNotFound(const Pattern *pattern, const char *str)
{
	return pattern->match(str).pos == NULL;
}

TEST_CASE("Pattern match", "[Fixed Pattern]")
{
	SECTION("Case sensitive fixed patterns", "[Pattern]")
	{
		Pattern *pf_abc = Pattern::create("abc", Pattern::FIXED, true);
		Pattern *pf_Abc = Pattern::create("Abc", Pattern::FIXED, true);
		Pattern *pf_aBc = Pattern::create("aBc", Pattern::FIXED, true);
		Pattern *pf_abC = Pattern::create("abC", Pattern::FIXED, true);
		Pattern *pf_ABC = Pattern::create("ABC", Pattern::FIXED, true);

		REQUIRE( patternMatch(    pf_abc, "abc", 0, 3 ) );
		REQUIRE( patternNotFound( pf_abc, "Abc") );
		REQUIRE( patternNotFound( pf_abc, "aBc") );
		REQUIRE( patternNotFound( pf_abc, "abC") );
		REQUIRE( patternNotFound( pf_abc, "ABC") );

		REQUIRE( patternNotFound( pf_Abc, "abc") );
		REQUIRE( patternMatch(    pf_Abc, "Abc", 0, 3 ) );
		REQUIRE( patternNotFound( pf_Abc, "aBc") );
		REQUIRE( patternNotFound( pf_Abc, "abC") );
		REQUIRE( patternNotFound( pf_Abc, "ABC") );

		REQUIRE( patternNotFound( pf_aBc, "abc") );
		REQUIRE( patternNotFound( pf_aBc, "Abc") );
		REQUIRE( patternMatch(    pf_aBc, "aBc", 0, 3 ) );
		REQUIRE( patternNotFound( pf_aBc, "abC") );
		REQUIRE( patternNotFound( pf_aBc, "ABC") );

		REQUIRE( patternNotFound( pf_abC, "abc") );
		REQUIRE( patternNotFound( pf_abC, "Abc") );
		REQUIRE( patternNotFound( pf_abC, "aBc") );
		REQUIRE( patternMatch(    pf_abC, "abC", 0, 3 ) );
		REQUIRE( patternNotFound( pf_abC, "ABC") );

		REQUIRE( patternNotFound( pf_ABC, "abc") );
		REQUIRE( patternNotFound( pf_ABC, "Abc") );
		REQUIRE( patternNotFound( pf_ABC, "aBc") );
		REQUIRE( patternNotFound( pf_ABC, "abC") );
		REQUIRE( patternMatch(    pf_ABC, "ABC", 0, 3 ) );

		REQUIRE( patternNotFound( pf_abc, "") );
		REQUIRE( patternNotFound( pf_abc, "ab") );
		REQUIRE( patternNotFound( pf_abc, "ac") );
	}

	SECTION("Case insensitive fixed patterns", "[Pattern]")
	{
		Pattern *pfc_abc = Pattern::create("abc", Pattern::FIXED, false);
		Pattern *pfc_Abc = Pattern::create("Abc", Pattern::FIXED, false);
		Pattern *pfc_aBc = Pattern::create("aBc", Pattern::FIXED, false);
		Pattern *pfc_abC = Pattern::create("abC", Pattern::FIXED, false);
		Pattern *pfc_ABC = Pattern::create("ABC", Pattern::FIXED, false);

		REQUIRE( patternMatch( pfc_abc, "abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abc, "Abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abc, "aBc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abc, "abC", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abc, "ABC", 0, 3 ) );

		REQUIRE( patternMatch( pfc_Abc, "abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_Abc, "Abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_Abc, "aBc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_Abc, "abC", 0, 3 ) );
		REQUIRE( patternMatch( pfc_Abc, "ABC", 0, 3 ) );

		REQUIRE( patternMatch( pfc_aBc, "abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_aBc, "Abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_aBc, "aBc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_aBc, "abC", 0, 3 ) );
		REQUIRE( patternMatch( pfc_aBc, "ABC", 0, 3 ) );

		REQUIRE( patternMatch( pfc_abC, "abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abC, "Abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abC, "aBc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abC, "abC", 0, 3 ) );
		REQUIRE( patternMatch( pfc_abC, "ABC", 0, 3 ) );

		REQUIRE( patternMatch( pfc_ABC, "abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_ABC, "Abc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_ABC, "aBc", 0, 3 ) );
		REQUIRE( patternMatch( pfc_ABC, "abC", 0, 3 ) );
		REQUIRE( patternMatch( pfc_ABC, "ABC", 0, 3 ) );

		REQUIRE( patternNotFound( pfc_abc, "") );
		REQUIRE( patternNotFound( pfc_abc, "ab") );
		REQUIRE( patternNotFound( pfc_abc, "ac") );
	}
}

