TARGET = snake

CC = g++

SOURCES = main.cpp

LIBS = -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

all:
	$(CC) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm $(TARGET)