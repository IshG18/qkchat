#Setting make variables
CXX := g++

#flags for warnings + optimization level + includes
CXXFLAGS := -std=c++17 -Wall -O2 \
	-I./asio-1.36.0/include \
	-I./core

LDFLAGS := -lws2_32 -lmswsock -luser32 -lkernel32
SRC := $(wildcard core/*.hpp)
MANIFEST  := client/dpi.manifest

all: server.exe client.exe

server.exe: server/server.cpp server/server_net.hpp $(SRC)
	$(CXX) $(CXXFLAGS) server/server.cpp -o server.exe $(LDFLAGS)

client.exe: client/client.cpp client/client_net.hpp client/client_console.h $(SRC)
	$(CXX) $(CXXFLAGS) -I./PDCurses -L./PDCurses/wincon client/client.cpp -o client.exe $(LDFLAGS) -lpdcurses
	mt.exe -manifest $(MANIFEST) -outputresource:client.exe;1

clean:
	del /f /q server.exe client.exe
	
# target: dependicies
# 	action