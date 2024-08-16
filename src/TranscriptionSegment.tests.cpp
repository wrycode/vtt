#include <catch2/catch_test_macros.hpp>

#include "TranscriptionSegment.h"
#include "Keyboard.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>

// Similar to direct Keyboard test, you must manually execute this by
// specifying "[.actually_type]" after the tests executable.
TEST_CASE("TranscriptionSegment with real typing", "[.actually_type]") {
  auto k = vtt::Keyboard();
  auto seg = vtt::TranscriptionSegment(k);

  std::string str = "Let's transcribe a full sentence.";
  std::string typed_string;

  std::thread typingThread([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    seg.update("let's transcribe");
    seg.update("Let's transcribe a");
    seg.applyFinalChange(str);
    // KEY_ENTER to submit the input
    k.press_key(28);
    k.release_key(28);
  });

  std::getline(std::cin, typed_string);
  typingThread.join();  // Wait for the typing thread to finish
  REQUIRE(typed_string == str);
}

// TEST_CASE("TranscriptionSegment") {
//   auto k = vtt::Keyboard();
//   auto seg = vtt::TranscriptionSegment(k);

//   // "Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي."
//   seg.update("12");
//   seg.update("Testin 1234");
//   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   // seg.update("Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي.");

//   seg.applyFinalChange("123");

//   REQUIRE(0 == 0);
// }
