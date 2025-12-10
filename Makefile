#Setting make variables
CXX := g++

#flags for warnings + optimization level + includes
CXXFLAGS := -std=c++17 -Wall -O2 \
	-I./asio-1.36.0/include \
	-I./core 

#flags to libraries
LDFLAGS := -lws2_32 -lmswsock -luser32 -lkernel32

all: server.exe client.exe

server.exe: server/server.cpp
	$(CXX) $(CXXFLAGS) server/server.cpp -o server.exe $(LDFLAGS)

client.exe: client/client.cpp
	$(CXX) $(CXXFLAGS) -I./PDCurses -L./PDCurses/wincon client/client.cpp -o client.exe $(LDFLAGS) -lpdcurses

clean:
	del /f /q server.exe client.exe
	
# target: dependicies
# 	action