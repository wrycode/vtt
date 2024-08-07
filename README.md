# Build instructions  * 
1. git clone --recurse-submodules --shallow-submodules https://github.com/wrycode/vtt.git

2. Install development and runtime dependencies for the main code: CMake, GCC, [ICU](https://icu.unicode.org/), [just](https://github.com/casey/just), Gstreamer, probably a few more I've forgotten

3. Install dependencies for the Cloud Speech-to-Text API C++ Client Library: https://github.com/googleapis/google-cloud-cpp/blob/main/doc/packaging.md#required-libraries

On Arch Linux, all the dependencies can be installed with: 
- `sudo pacman -S cmake abseil-cpp grpc curl nlohmann-json just icu`
- manually install [google-crc32c](https://aur.archlinux.org/packages/google-crc32c) from the AUR

4. Run `just` to build and compile

# Enabling virtual keyboard

Keys are typed using the [uinput](https://kernel.org/doc/html/v4.12/input/uinput.html) kernel interface, exposed in /dev/uinput. The program needs to have permissions to write to /dev/uinput. The easiest way is to chmod /dev/uinput, but there is probably a smarter way with udev.


# Unicode characters

For languages other than English, you must have [IBus](https://wiki.archlinux.org/title/IBus). It may be enabled/installed already. You can test if it is installed by typing Ctrl+Shift+U (then release), then 2+7+0+5. You should see: ✅
