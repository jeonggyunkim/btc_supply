make:
	g++ -O2 -o analyze analyze.cpp sha256.cpp
mem:
	g++ -O2 -o analyze_memory analyze_memory_save.cpp sha256.cpp
