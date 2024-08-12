#include <catch2/catch_test_macros.hpp>

#include "TranscriptionSegment.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>

TEST_CASE("TranscriptionSegment") {
  auto seg = vtt::TranscriptionSegment(0);

  seg.update("testin 123");
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  seg.update("Testin 1234");
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  seg.update("Testing 1234");

  seg.applyFinalChange("Testin 12345!");

  REQUIRE(0 == 0);
}
