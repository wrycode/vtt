#include <vector>
#include <string>
#include <cstdint>


namespace vtt {
  namespace test {


    /* Convert a utf-8 encoded string to a sequence Unicode code points,
       one uint32_t per character. Assume str is encoded correctly.

       Some background reading: https://www.tsmean.com/articles/encoding/unicode-and-utf-8-tutorial-for-dummies/

       This was originally used when I was building code to type
       text. I know it works correctly to create code points for iBus,
       so we use it in tests, for example to check the validity of
       runes or test type_unicode.
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

  }
}
