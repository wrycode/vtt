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

  seg.applyChange("testin 123");
  seg.applyChange("testin 1234");
  seg.applyChange("Testin 1234");
    seg.applyChange("testin 123");
  seg.applyChange("testin 1234");
  seg.applyChange("Testin 1234");
  seg.applyChange("testin 123");
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));  

  seg.applyFinalChange("Testin 12345!");
  
  REQUIRE(0 == 0);
}
