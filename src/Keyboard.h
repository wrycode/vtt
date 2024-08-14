#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <cstdint>

namespace vtt {
  class Keyboard {
  public:
    Keyboard();
    ~Keyboard();
    void type(uint32_t code);	// Type the character representing the Unicode code point
    const void ndeletes(int n);	// Type "delete" n times

  private:
    int fd;
    constexpr void press_key(__u16 code);
    constexpr void release_key(__u16 code);
    constexpr void send_event(int fd, __u16 type, __u16 code, int val);
  };
}


// old
/* namespace vtt { */
/*   void press_key(int fd, __u16 code); */
/*   int get_keyboard_fd(); */
/*   void close_keyboard(int fd); */
/*   void type_unicode(int fd, uint32_t code); */
/*   void ndeletes(int fd, int n); */
/* } */

#endif // KEYBOARD_H_
