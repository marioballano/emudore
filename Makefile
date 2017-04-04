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
	cd build;cmake -DCMAKE_TOOLCHAIN_FILE=~/emsdk/emscripten/incoming/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build;make
wasm:
	rm -rf build/*
	cd build;cmake -DCMAKE_TOOLCHAIN_FILE=~/emsdk/emscripten/incoming/cmake/Modules/Platform/Emscripten.cmake -DWASM=1 -DCMAKE_BUILD_TYPE=Release ..
	cd build;make
