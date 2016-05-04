release:
	rm -rf build/*
	cd build;cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build;make
debug:
	rm -rf build/*
	cd build;cmake -DCMAKE_BUILD_TYPE=Debug ..
	cd build;make
js:
	rm -rf build/*
	cd build;cmake -DCMAKE_TOOLCHAIN_FILE=~/emsdk_portable/emscripten/master/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build;make
