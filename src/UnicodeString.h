#ifndef VTT_UNICODE_STRING_H_
#define VTT_UNICODE_STRING_H_

#include <string>
#include <vector>
#include <cstdint>

#include <unicode/brkiter.h>

namespace vtt {

  // Sequence of one or more Unicode code points that form a
  // user-perceived character. These are also sometimes referred to as
  // grapheme clusters.
  using Rune = std::vector<uint32_t>;

  /* vtt::UnicodeString is an immutable string type that uses the ICU
     library internally to split up the string into a sequence of
     Runes.

     TODO: are language codes needed? An optional language code can be
     supplied and the ICU BreakIterator "may be specialized in some
     way for that locale"

     The original UTF8-encoded std::string and sequence of Runes is
     available for calculations outside the class. For convenience we
     also overwrite operator<<.
  */
  class UnicodeString {
  public:
    UnicodeString(const std::string& str, icu::BreakIterator* b);
    const std::string std_string;
    std::vector<Rune> runes;    
    friend std::ostream& operator<<(std::ostream& os, const UnicodeString& obj);
  };
}

#endif /* VTT_UNICODE_STRING_H_ */
