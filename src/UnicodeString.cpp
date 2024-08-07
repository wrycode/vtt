#include "UnicodeString.h"

// NOTE: this is the only file that uses ICU. In Transcription.cpp you can safely use UnicodeString 
// #include <unicode/unistr.h>
// #include <unicode/brkiter.h>
#include <iostream>
#include <string>


namespace vtt {
  // Constructor
  UnicodeString::UnicodeString(const std::string& str, const std::string lcode="en") {
      std_string = str;
      // TODO: split up into runes
      // set up demo rune
      // try {
	// 👨‍👩‍👧‍👦
        // Rune family{0x1F468, 0x200D, 0x1F469, 0x200D, 0x1F467, 0x200D, 0x1F466};
        // for (const auto& value : family) {
        //     std::cout << value << " ";
        // }
        // std::cout << std::endl;

	// std::vector<Rune> runes = std::vector<Rune>{family};
	
    // } catch (const std::exception& e) {
    //     std::cerr << "Exception: " << e.what() << '\n';
    // }
      // iterate through graphemes in str and create runes vector
    };

  // Overload << operator
  std::ostream& operator<<(std::ostream& os, const UnicodeString& obj) {
    os << obj.std_string;
    return os;
  }
}


