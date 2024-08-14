#include <catch2/catch_test_macros.hpp>

#include "TranscriptionSegment.h"
#include "Keyboard.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>

TEST_CASE("TranscriptionSegment") {
  auto k = vtt::Keyboard();
  auto seg = vtt::TranscriptionSegment(k);

  // "Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي."
  seg.update("12");
  seg.update("Testin 1234");
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // seg.update("Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي.");

  seg.applyFinalChange("123");

  REQUIRE(0 == 0);
}
