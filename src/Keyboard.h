#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <unordered_map>
#include <vector>

#include <cstdint>


namespace vtt {

  extern const std::unordered_map<uint32_t, std::vector<int>> ascii_keys;
  
  class Keyboard {
  public:
    Keyboard();
    virtual ~Keyboard();
    virtual void type(uint32_t code);	// Type the character representing the Unicode code point
    virtual const void ndeletes(int n);	// Type "delete" n times
    virtual void press_key(__u16 code);
    virtual void release_key(__u16 code);

  private:
    int fd;
    constexpr void send_event(int fd, __u16 type, __u16 code, int val);
  };
}

#endif // KEYBOARD_H_
