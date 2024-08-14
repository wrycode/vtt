#include <catch2/catch_test_macros.hpp>
// #include <catch2/trompeloeil.hpp>

#include <unistd.h>

#include "Keyboard.h"
#include "test_common.h"
#include <fstream>
#include <cstring>
#include <iostream>

TEST_CASE("type_input") {
  
  auto k = vtt::Keyboard();
  std::string str = "123";
  // ""Russian: Привет, мир! Это тестовый текст.(Hello, world! This is test text.)\nArabic: مرحبا بك في العالم! هذا نص تجريبي.";
  std::vector<uint32_t> code_points = vtt::test::convertToUnicode(str);
  for (size_t i = 0; i < code_points.size(); ++i)
    {
      k.type(code_points[i]);
    }

  // =======================================
  // int fake_fd[2];
  // pipe(fake_fd); // fake_fd[1] is the writing end, fake_fd[0] is the reading end

  // // Use fake_fd[1] in place of your /dev/uinput file descriptor

  // // int fd  = vtt::get_keyboard_fd();
  // // Represents one of potentially multiple UTF-8 strings that would be streamed from the Google API

  // std::string str = "T"; // ASCII string

  // // この壁 何のデザインでしょうか  
  // // Inefficient, but easy reason about. In the future, may use a
  // // utf-8 aware library to iterate through str directly or convert to
  // // a wide with std::codecvt_utf8:

  // vtt::ndeletes(fake_fd[1], 7);
  // // Give userspace some time to read the events before we destroy the
  // // device with UI_DEV_DESTROY.

  // char buffer[1024];
  // ssize_t bytesRead = read(fake_fd[0], buffer, 1024);
  // if (bytesRead > 0) {
  //   // Process the data in the buffer
  //   std::string data(buffer, bytesRead);
  //   std::cout << data << "\n";
  // }


  // vtt::close_keyboard(fd);

  REQUIRE(0 == 0);
  
}
