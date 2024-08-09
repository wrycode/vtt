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
  
  REQUIRE(0 == 0);
}
