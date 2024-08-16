#include <string>
#include <iostream>

#include <unicode/brkiter.h>

#include "TranscriptionSegment.h"
#include "Keyboard.h"

// Third-Party library headers
// #include <plog/Log.h>
// #include <plog/Initializers/RollingFileInitializer.h>

#include "src/UnicodeString.h"

int main() {

  auto k = vtt::Keyboard();
  auto seg = vtt::TranscriptionSegment(k);
  std::string str = "Let's transcribe a full sentence.";

  std::this_thread::sleep_for(std::chrono::seconds(1));  
  
  seg.update("let's transcribe");
  std::this_thread::sleep_for(std::chrono::seconds(1));  
    seg.update("Let's transcribe a");
  std::this_thread::sleep_for(std::chrono::seconds(2));    
    seg.applyFinalChange(str);

  return 0;
}
 
