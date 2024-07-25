# compile project
build: google
	mkdir -p build && cmake . -B build && make -C build
# compile Google Cloud Speech-to-Text API C++ Client Library
google:
	mkdir -p build && cmake -S extern/google-cloud-cpp -B build/google-cmake \
	-DBUILD_TESTING=OFF \
	-DGOOGLE_CLOUD_CPP_ENABLE_EXAMPLES=OFF \
	-DGOOGLE_CLOUD_CPP_ENABLE=speech \
	-DCMAKE_INSTALL_PREFIX=build/google-install && \
	cmake --build build/google-cmake -- -j "$(nproc)" && \
	cmake --build build/google-cmake --target install
