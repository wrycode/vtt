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
      
      const UChar* buffer = charstring.getBuffer();
      uint32_t len = charstring.length();
      for (uint32_t i = 0; i < len; ++i) {
	r.push_back(buffer[i]);
      }
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
