MY_PATH = "/c/Users/cerne/Documents/Dev/SDL_Test/"

TARGET = ../snake

CC = g++

SOURCES = main.cpp

INCLUDE = -I ../src/include -I ../src/include/SDL2

LIBS = -L ../src/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -B ../src/bin

all:
	$(CC) $(INCLUDE) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm $(TARGET)