default:
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/ripemd160.o -c hash/ripemd160.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/sha256.o -c hash/sha256.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/ripemd160_sse.o -c hash/ripemd160_sse.cpp
	g++ -m64 -mssse3 -Wno-write-strings -O2 -o hash/sha256_sse.o -c hash/sha256_sse.cpp
	g++ -O3 -o minikeyg minikeyg.cpp hash/ripemd160.o hash/ripemd160_sse.o hash/sha256.o hash/sha256_sse.o -lpthread

