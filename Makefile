CC = gcc
SRC = SRC/main.c SRC/visualize.c SRC/sorting_algos.c
INCLUDE = -Iinclude -I"C:/SDL3/x86_64-w64-mingw32/include"
LIB = -L"C:/SDL3/x86_64-w64-mingw32/lib" -lSDL3
TARGET = main.exe

all:
	$(CC) $(SRC) -o $(TARGET) $(INCLUDE) $(LIB) -mconsole

clean:
	if exist $(TARGET) del /Q $(TARGET)
