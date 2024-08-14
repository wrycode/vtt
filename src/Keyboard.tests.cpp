#include <catch2/catch_test_macros.hpp>
#include <catch/fakeit.hpp>

#include "Keyboard.h"
#include "test_common.h"
#include <linux/input-event-codes.h>

#include <thread>
#include <fstream>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <vector>

using namespace fakeit; // https://github.com/eranpeer/FakeIt/wiki/Quickstart

// Tests in this file are pretty simple and are just verify at a basic
// level that we can create the keyboard and type a few keys. The
// class receives more extensive testing by proxy when we test
// TranscriptionSegment.
TEST_CASE("Keyboard") {

  std::string str = "123😀"; // 😀 is U+1F600

  // We're mocking only press_key() and release_key() in the Keyboard
  // class so we can verify that the correct keys are being pressed
  // TODO: override constructor
  vtt::Keyboard keeb;
  Mock<vtt::Keyboard> mock(keeb);

  std::vector<__u16> keycodes;
  std::vector<__u16> expected_keycodes = { KEY_1, KEY_2, KEY_3,
     KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_U,
     KEY_0, KEY_0, KEY_0,
     KEY_1, KEY_F, KEY_6, KEY_0, KEY_0, KEY_ENTER};

  When(Method(mock,press_key)).AlwaysDo([&keycodes](auto code){ keycodes.push_back(code);});
  When(Method(mock, release_key)).AlwaysReturn();
  
  vtt::Keyboard &k = mock.get();

  std::vector<uint32_t> code_points = vtt::test::convertToUnicode(str);
  for (size_t i = 0; i < code_points.size(); ++i)
    {
      k.type(code_points[i]);
    }
  Verify(Method(mock,press_key)).Exactly(15);
  
  REQUIRE(keycodes == expected_keycodes);
  
}

// Won't execute by default (since it actually starts typing), see
// https://github.com/catchorg/Catch2/blob/devel/docs/command-line.md#specifying-which-tests-to-run
TEST_CASE("Keyboard with real typing", "[.actually_type]") { 

  vtt::Keyboard keeb;
  std::string str = "👮‍♀️Tëst.👨‍👩‍👦🇺🇸नीநி!";
  std::string typed_string;
  std::vector<uint32_t> code_points = vtt::test::convertToUnicode(str);

  std::thread typingThread([&]() {
    // std::this_thread::sleep_for(std::chrono::milliseconds(200));    
        for (size_t i = 0; i < code_points.size(); ++i) {
	  keeb.type(code_points[i]);
        };
	// KEY_ENTER to submit the input
	keeb.press_key(28);
	keeb.release_key(28);
    });

  std::getline(std::cin, typed_string);

  typingThread.join();  // Wait for the typing thread to finish
  // for (size_t i = 0; i < code_points.size(); ++i)
  //   {
  //     keeb.type(code_points[i]);
  //   }

  REQUIRE(typed_string == str);
  
}


