#include <catch2/catch_test_macros.hpp>

#include <string>
#include <iostream>
#include <unicode/brkiter.h>

#include "UnicodeString.h"
#include "TranscriptionSegment.h"
#include "test_common.h"


TEST_CASE("UnicodeString") {

  auto b = vtt::setup_BreakIterator();

  SECTION("UnicodeString constructor works") {
    REQUIRE(vtt::UnicodeString("👮‍♀️Tëst.👨‍👩‍👦🇺🇸नीநி!", b).std_string == "👮‍♀️Tëst.👨‍👩‍👦🇺🇸नीநி!" );
    REQUIRE(vtt::UnicodeString("abcd", b).std_string != "abc" );
    REQUIRE(vtt::UnicodeString("", b).std_string == "" );
  }

  SECTION("UnicodeString runes are correct") {
    auto ustring = vtt::UnicodeString("👮‍♀️Tëst.👨‍👩‍👦🇺🇸नीநி!", b);

    std::vector<vtt::Rune> correct_runes = {
      { 0x1f46e, 0x200d, 0x2640, 0xfe0f },
      { 0x54 },
      { 0xeb },
      { 0x73 },
      { 0x74 },
      { 0x2e },
      { 0x1f468, 0x200d, 0x1f469, 0x200d, 0x1f466 },
      { 0x1f1fa, 0x1f1f8 },
      { 0x928, 0x940 },
      { 0xba8, 0xbbf },
      { 0x21 } };

    REQUIRE(ustring.runes == correct_runes);

    // Manually decode the UTF8 string, verify that code points match runes
    auto ustring1 = vtt::UnicodeString("👮‍♀️", b);
    

    std::vector<uint32_t> code_points;

    // flatten Runes into a sequence of code points
    for (vtt::Rune& r : ustring1.runes) {
      for (uint32_t code_point : r) {
	code_points.push_back(code_point);
      };
    };

    // Code points should be identical to the ones we calculate separately
    REQUIRE(code_points == vtt::test::convertToUnicode(ustring1.std_string));
  }
  
}
