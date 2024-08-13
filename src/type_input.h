#ifndef TYPE_INPUT_H_
#define TYPE_INPUT_H_










namespace vtt {
  int get_keyboard_fd();
  void close_keyboard(int fd);
  void type_unicode(int fd, uint32_t code);
  void ndeletes(int fd, int n);
}

#endif // TYPE_INPUT_H_
