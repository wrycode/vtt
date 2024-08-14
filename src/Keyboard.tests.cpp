#include <catch2/catch_test_macros.hpp>
#include <catch/fakeit.hpp>

// #include <unistd.h>

#include "Keyboard.h"
#include "test_common.h"

#include <fstream>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <vector>

// https://github.com/eranpeer/FakeIt/wiki/Quickstart
using namespace fakeit;

TEST_CASE("type_input") {

  vtt::Keyboard keeb;
  
  // Mock setup
  Mock<vtt::Keyboard> mock(keeb);

  std::vector<__u16> keycodes;

  std::vector<__u16> expected_keycodes = {2, 3, 4};
    // { vtt::ascii_keys.find('1')[0],
    //   vtt::ascii_keys.find('2')[0],
    //   vtt::ascii_keys.find('3')[0]
    // };
    
  When(Method(mock,press_key)).AlwaysDo([&keycodes](auto code){ keycodes.push_back(code);});
  When(Method(mock, release_key)).AlwaysReturn();
  
  // Fetch the mock instance.
  vtt::Keyboard &k = mock.get();

  std::string str = "123";
  std::vector<uint32_t> code_points = vtt::test::convertToUnicode(str);
  for (size_t i = 0; i < code_points.size(); ++i)
    {
      k.type(code_points[i]);
    }

  Verify(Method(mock,press_key)).Exactly(3);

  // std::cout << keycodes << "<- keycodes \n";

  
  REQUIRE(keycodes == expected_keycodes);
  
}
