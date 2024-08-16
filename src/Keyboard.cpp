#include "Keyboard.h"

// Headers for Linux input handling
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>

// Standard C libraries for errors and io
#include <errno.h>		// c99
#include <stdio.h>		// c99
#include <unistd.h>		// posix
#include <fcntl.h>		// posix

#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>
#include <cstring>

namespace vtt
{
  // All keys used by this program, so we can enable them for the virtual keyboard
  constexpr __u16 keys_used[] = { KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_LEFTSHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT, KEY_KPASTERISK, KEY_LEFTALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_K, KEY_K, KEY_K, KEY_KPMINUS, KEY_K, KEY_K, KEY_K, KEY_KPPLUS, KEY_K, KEY_K, KEY_K, KEY_K, KEY_KPDOT };

  // Mapping of ASCII to their typed representation with uinput event
  // codes. Multiple keys are interpreted as a single key combination.
  const std::unordered_map<uint32_t, std::vector<int>> ascii_keys = {
    {'1', {KEY_1}},
    {'2', {KEY_2}},
    {'3', {KEY_3}},
    {'4', {KEY_4}},
    {'5', {KEY_5}},
    {'6', {KEY_6}},
    {'7', {KEY_7}},
    {'8', {KEY_8}},
    {'9', {KEY_9}},
    {'0', {KEY_0}},

    // symbols
    {'\n', {KEY_ENTER}},
    {';', {KEY_SEMICOLON}},
    {'\'', {KEY_APOSTROPHE}},
    {'`', {KEY_GRAVE}},
    {'\\', {KEY_BACKSLASH}},
    {',', {KEY_COMMA}},
    {'.', {KEY_DOT}},
    {'*', {KEY_KPASTERISK}},
    {' ', {KEY_SPACE}},
    {'-', {KEY_KPMINUS}},
    {'/', {KEY_SLASH}},
    {'!', {KEY_LEFTSHIFT, KEY_1}},

    // lowercase letters
    {'a', {KEY_A}},
    {'b', {KEY_B}},
    {'c', {KEY_C}},
    {'d', {KEY_D}},
    {'e', {KEY_E}},
    {'f', {KEY_F}},
    {'g', {KEY_G}},
    {'h', {KEY_H}},
    {'i', {KEY_I}},
    {'j', {KEY_J}},
    {'k', {KEY_K}},
    {'l', {KEY_L}},
    {'m', {KEY_M}},
    {'n', {KEY_N}},
    {'o', {KEY_O}},
    {'p', {KEY_P}},
    {'q', {KEY_Q}},
    {'r', {KEY_R}},
    {'s', {KEY_S}},
    {'t', {KEY_T}},
    {'u', {KEY_U}},
    {'v', {KEY_V}},
    {'w', {KEY_W}},
    {'x', {KEY_X}},
    {'y', {KEY_Y}},
    {'z', {KEY_Z}},

    // uppercase letters
    {'A', {KEY_LEFTSHIFT, KEY_A}},
    {'B', {KEY_LEFTSHIFT, KEY_B}},
    {'C', {KEY_LEFTSHIFT, KEY_C}},
    {'D', {KEY_LEFTSHIFT, KEY_D}},
    {'E', {KEY_LEFTSHIFT, KEY_E}},
    {'F', {KEY_LEFTSHIFT, KEY_F}},
    {'G', {KEY_LEFTSHIFT, KEY_G}},
    {'H', {KEY_LEFTSHIFT, KEY_H}},
    {'I', {KEY_LEFTSHIFT, KEY_I}},
    {'J', {KEY_LEFTSHIFT, KEY_J}},
    {'K', {KEY_LEFTSHIFT, KEY_K}},
    {'L', {KEY_LEFTSHIFT, KEY_L}},
    {'M', {KEY_LEFTSHIFT, KEY_M}},
    {'N', {KEY_LEFTSHIFT, KEY_N}},
    {'O', {KEY_LEFTSHIFT, KEY_O}},
    {'P', {KEY_LEFTSHIFT, KEY_P}},
    {'Q', {KEY_LEFTSHIFT, KEY_Q}},
    {'R', {KEY_LEFTSHIFT, KEY_R}},
    {'S', {KEY_LEFTSHIFT, KEY_S}},
    {'T', {KEY_LEFTSHIFT, KEY_T}},
    {'U', {KEY_LEFTSHIFT, KEY_U}},
    {'V', {KEY_LEFTSHIFT, KEY_V}},
    {'W', {KEY_LEFTSHIFT, KEY_W}},
    {'X', {KEY_LEFTSHIFT, KEY_X}},
    {'Y', {KEY_LEFTSHIFT, KEY_Y}},
    {'Z', {KEY_LEFTSHIFT, KEY_Z}},
  };

  /* Send an event (key press/release, etc, defined in input-event-codes.h) to /dev/uinput
   * @param fd The file descriptor for the uinput device.
   * @param type The type of the event. Could be EV_KEY for key press/release events, EV_SYN to synchronize, and others
   * @param code The key code. For instance, KEY_A, KEY_ENTER, etc.
   * @param val The value for the event. If type is EV_KEY, then 0 is for KEY_RELEASE, and 1 is for KEY_PRESS.
   */
  constexpr void Keyboard::send_event(int fd, __u16 type, __u16 code, int val)
  {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* The time value isn't important when creating synthetic events.
       The kernel processes events immediately as they are generated.
    */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    // printf("Sending event: fd=%d type=%d, code=%d, value=%d\n", fd, type, code, val);
    ssize_t bytes_written = write(fd, &ie, sizeof(ie));
    // std::cout << "Bytes written: " << bytes_written << "\n";
    if (bytes_written == -1) {
      perror("write");
    }
  }

  // Helper function to press a key
  void Keyboard::press_key(__u16 code) {
    send_event(fd, EV_KEY, code, 1);  // Key press
    send_event(fd, EV_SYN, SYN_REPORT, 0); // report the event
    usleep(10000);
  }

  // Helper function to release a key
  void Keyboard::release_key(__u16 code) {
    send_event(fd, EV_KEY, code, 0);  // Key release
    send_event(fd, EV_SYN, SYN_REPORT, 0);
    usleep(10000);
  }

  Keyboard::Keyboard() {
    // request a virtual device from the uinput kernel subsystem; see https://kernel.org/doc/html/v4.12/input/uinput.html
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0)
      {
	// PLOGE << "Failed to open /dev/uinput";
	perror("Failed to open /dev/uinput");
	// TODO: properly manage failure
      }

    // enable key press/release events
    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    // enable needed keys
    for (size_t i = 0; i < (sizeof(keys_used)/sizeof(__u16)); ++i)
      {
	ioctl(fd, UI_SET_KEYBIT, keys_used[i]);
      }

    struct uinput_setup usetup;

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Example device");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    // Text below is from
    // https://kernel.org/doc/html/v4.12/input/uinput.html


    /*
     * On UI_DEV_CREATE the kernel will create the device node for this
     * device. We are inserting a pause here so that userspace has time
     * to detect, initialize the new device, and can start listening to
     * the event, otherwise it will not notice the event we are about
     * to send. This pause is only needed in our example code!
     */
    // std::cout << "initializing keyboard with fd: " << fd << "\n";

    // Upon testing, the sleep is not actually necessary for the
    // program to work because there is some lag between creating the
    // keyboard and sending events, but it is necessary for unit tests
    // where we use the keyboard immediately, plus better safe than sorry.

    usleep(100000);
    return;
  };

  Keyboard::~Keyboard() {
    sleep(1);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
  };

  /* Type the character representing the Unicode code point. Characters
     can be typed directly into the keyboard if they are in ascii_keys,
     otherwise we use iBus.

     Unicode key entry with iBus: Ctrl+Shift+u (then release) followed
     by the hexadecimal Unicode code point (e.g. 2 + 0 + b + 9) + Enter
     or Space

     * @param code The Unicode code for the character to be typed.
     */
  void Keyboard::type(uint32_t code) {
    // std::cout << "trying to type " << code << "\n";

    // Check if we can type some ASCII characters directly
    auto iterator = ascii_keys.find(code);
    // std::vector<int> ascii = ascii_keys.find(code);
    if (iterator != ascii_keys.end())
      {
	// std::cout << "writing ascii\n";
	std::vector<int> keys = iterator->second;
	for (int i = 0; i < keys.size(); i++) {
	  press_key(keys[i]);
	}
	for (int i = 0; i < keys.size(); i++) {
	  release_key( keys[i]);
	}
      }
    else	// Character is not in our ascii map; type Unicode code point directly
      {
	// std::cout << "writing unicode\n";

	// Press Ctrl + Shift + U
	press_key(KEY_LEFTCTRL);
	press_key(KEY_LEFTSHIFT);
	press_key(KEY_U);

	// Release keys
	release_key( KEY_U);
	release_key( KEY_LEFTCTRL);
	release_key( KEY_LEFTSHIFT);

	// Array for mapping hexadecimal characters to keycodes
	__u16 key_map[] = {
	  KEY_0, KEY_1, KEY_2, KEY_3,
	  KEY_4, KEY_5, KEY_6, KEY_7,
	  KEY_8, KEY_9, KEY_A, KEY_B,
	  KEY_C, KEY_D, KEY_E, KEY_F
	};

	// Enter the Unicode code point
	char unicode_hex[9]; // 8 hex digits plus null terminator.
	sprintf(unicode_hex, "%08x", code);

	for (int i = 0; i < 8; i++) {
	  // Get the index of the hex character to access corresponding keycode
	  int index = (unicode_hex[i] <= '9') ? (unicode_hex[i] - '0') : (unicode_hex[i] - 'a' + 10);
	  __u16 keycode = key_map[index];

	  press_key(keycode);
	  release_key( keycode);
	}

	// Press Enter to confirm
	press_key(KEY_ENTER);
	release_key( KEY_ENTER);
      }
  };

  // type backspace ntimes, deleting n characters of input.
  const void Keyboard::ndeletes(int n) {
    for (int i = 0; i < n; i++) {
      press_key(KEY_BACKSPACE);
      release_key( KEY_BACKSPACE);
    };
  };
}
