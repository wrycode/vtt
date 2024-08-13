#include <string>
#include <iostream>

#include <unicode/brkiter.h>

// Third-Party library headers
// #include <plog/Log.h>
// #include <plog/Initializers/RollingFileInitializer.h>

#include "src/UnicodeString.h"

int main() {

  // Create a BreakIterator. They are costly to create but can be reused for different strings
  // https://unicode-org.github.io/icu/userguide/boundaryanalysis/
  icu::BreakIterator* b;
  auto lcode = icu::Locale::getDefault();
  UErrorCode status = U_ZERO_ERROR;
  b = icu::BreakIterator::createCharacterInstance(lcode, status);
  if (U_FAILURE(status)) {
    std::cerr << "Failed to create sentence break iterator. Status: " << u_errorName(status) << std::endl;
    return 1;
  }

  auto ustr = vtt::UnicodeString("👮‍♀️Tëst.👨‍👩‍👦🇺🇸नीநி!", b);
  
  std::cout << "ustr is: " << ustr << "\n";

  
  for (size_t i = 0; i < ustr.runes.size(); ++i) {
    std::cout << "rune at position " << std::dec << i << ": ";  
    // access element at index i
    auto r = ustr.runes[i];
    for(uint32_t c : r) {
      std::cout << std::hex << c;  
      std::cout << " ";  
    }
    std::cout << "\n";
  };

  return 0;
}
