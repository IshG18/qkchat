#Setting make variables
CXX := g++

#flags for warnings + optimization level + includes
CXXFLAGS := -std=c++17 -Wall -O2 \
	-I./asio-1.36.0/include \
	-I./core \
	-I./blake3/include

LDFLAGS := -lws2_32 -lmswsock -luser32 -lkernel32 -lblake3
SRC := $(wildcard core/*.hpp)
MANIFEST  := client/dpi.manifest

LDPATHS := -L./blake3/lib

all: server client

server: server/server.cpp server/server_net.hpp $(SRC)
	$(CXX) $(CXXFLAGS) $(LDPATHS) server/server.cpp -o server.exe $(LDFLAGS)

client: client/client.cpp client/client_net.hpp client/client_console.h $(SRC)
	$(CXX) $(CXXFLAGS) $(LDPATHS) -I./PDCurses -L./PDCurses/wincon client/client.cpp -o client.exe $(LDFLAGS) -lpdcurses
	mt.exe -manifest $(MANIFEST) -outputresource:client.exe;1

test: test.cpp $(SRC) 
	$(CXX) $(CXXFLAGS) $(LDPATHS) test.cpp -o test.exe $(LDFLAGS)

clean:
	del /f /q server.exe client.exe
	
# target: dependicies
# 	action