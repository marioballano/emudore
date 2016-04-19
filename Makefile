release:
	rm -rf build/*
	cd build;cmake -DCMAKE_BUILD_TYPE=Release ..
	cd build;make
debug:
	rm -rf build/*
	cd build;cmake -DCMAKE_BUILD_TYPE=Debug ..
	cd build;make    
