#include <catch2/catch_test_macros.hpp>

#include "type_input.h"
#include "test_common.h"

TEST_CASE("type_input") {

  int fd  = vtt::get_keyboard_fd();
    // Represents one of potentially multiple UTF-8 strings that would be streamed from the Google API
  // std::u8string str = u8"Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي.";
  std::string str = "Test Here. Now a Russian word: Привет. Now, let's add some punctuation to our string!"; // ASCII string

  // この壁 何のデザインでしょうか  
  // Inefficient, but easy reason about. In the future, may use a
  // utf-8 aware library to iterate through str directly or convert to
  // a wide with std::codecvt_utf8:
  std::vector<uint32_t> code_points = vtt::test::convertToUnicode(str);
  for (size_t i = 0; i < code_points.size(); ++i)
    {
      vtt::type_unicode(fd, code_points[i]);
    }

  vtt::ndeletes(fd, 7);
  // Give userspace some time to read the events before we destroy the
  // device with UI_DEV_DESTROY.

  vtt::close_keyboard(fd);



  REQUIRE(0 == 0);
  
}
