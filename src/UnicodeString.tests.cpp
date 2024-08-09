#include <catch2/catch_test_macros.hpp>

#include <string>
#include <iostream>
#include <unicode/brkiter.h>

#include "UnicodeString.h"

icu::BreakIterator* setup_BreakIterator() {
  icu::BreakIterator* b;
  auto lcode = icu::Locale::getDefault();
  UErrorCode status = U_ZERO_ERROR;
  b = icu::BreakIterator::createCharacterInstance(lcode, status);
  if (U_FAILURE(status)) {
    std::cerr << "Failed to create sentence break iterator. Status: " << u_errorName(status) << std::endl;
  }
  return b;
};

/* Convert a utf-8 encoded string to a sequence Unicode code points,
   one uint32_t per character. Assume str is encoded correctly.

   Some background reading: https://www.tsmean.com/articles/encoding/unicode-and-utf-8-tutorial-for-dummies/

   This was originally used when I was building code to type text. I
   know it works correctly to create code points for iBus, so we use
   it below to check the validity of our Runes.
*/
std::vector<uint32_t> convertToUnicode(std::string const& str)
{
  std::vector<uint32_t> unicode_values;
  char32_t ch = 0;
  int bytes_in_ch = 0;

  for (unsigned char c : str)
    {
      if (c <= 0b01111111) // 1-byte char (ASCII)
	ch = c, bytes_in_ch = 1;
      else if ((c & 0b11100000) == 0b11000000) // 2-byte char
	ch = c & 0b00011111, bytes_in_ch = 2;
      else if ((c & 0b11110000) == 0b11100000) // 3-byte char
	ch = c & 0b00001111, bytes_in_ch = 3;
      else if ((c & 0b11111000) == 0b11110000) // 4-byte char
	ch = c & 0b00000111, bytes_in_ch = 4;
      else // continuation bytes
	ch = (ch << 6) | (c & 0b00111111);
      if (--bytes_in_ch <= 0)
	unicode_values.push_back(ch), bytes_in_ch = 0;
    }
  return unicode_values;
}

TEST_CASE("UnicodeString") {

  auto b = setup_BreakIterator();

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
    REQUIRE(code_points == convertToUnicode(ustring1.std_string));
  }
  
}
