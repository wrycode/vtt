#include "UnicodeString.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <iostream>
#include <string>
namespace vtt {

  // Constructor
  UnicodeString::UnicodeString(const std::string& str, icu::BreakIterator* b)
    : std_string(str)
  {
    icu::UnicodeString ustring = icu::UnicodeString::fromUTF8(str);

    // We'll use the icu::BreakIterator to break the string into
    // perceived characters
    b->setText(ustring);

    int32_t pos = 0;
    for (pos = b->first(); pos != icu::BreakIterator::DONE;) {
      auto old_pos = pos;
      pos = b->next();

      icu::UnicodeString charstring = ustring.tempSubString(old_pos, pos - old_pos);

      // Push each code point to a Rune
      Rune r;

      // charstring is in the ICU default of UTF-16, we need to
      // convert to 32 or else we'll incorrectly store two code points
      // for each UTF-16 surrogate pair
      int length = charstring.countChar32();
      UErrorCode status = U_ZERO_ERROR;
      UChar32 buffer[length];
      charstring.toUTF32(buffer, length, status);

      for (int i = 0; i < length; ++i) {
	r.push_back(buffer[i]);
      };

      runes.push_back(r);
    };
      
  // Remove the last element, which is empty
  runes.pop_back();
  };
  // Could probably make this code a lot cleaner, possibly more efficient. But it works.↑↑↑
  
  // Overload << operator
  std::ostream& operator<<(std::ostream& os, const UnicodeString& obj) {
    os << obj.std_string;
    return os;
  }
}
