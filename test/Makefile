# This is a good referal for makefile usage with Obliviengine

CC=gcc
CFLAGS= -g -O2 -I../include -I../include/SDL/include -I../shaders

UNAME_S := $(shell uname -s)
# setup required linker flags for 
LDFLAGS= -lm -ldl -lpthread
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -L../lib/lin -lOE -lSDL2 -lX11 -lGL 
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -L../lib/mac -lOE -lSDL2 -framework Cocoa -framework OpenGL
endif
ifeq ($(OS),Windows_NT)

# If you are using OpenXR do not forget to use
#  OPENXRLIB= C:/msys64/mingw64/lib (or wherever you have it installed)
#  LDFLAGS += -L$(OPENXRLIB) -lopenxr_loader.dll

    LDFLAGS += -L../lib -lOE -lSDL2 -lgdi32 -lopengl32
endif


COMMON_C= *.c

COMMON_O= *.o

OUT= oetest

all: build

build:
	$(CC) $(CFLAGS) -c $(COMMON_C)
	$(CC) $(CFLAGS) $(COMMON_O) -o $(OUT) $(LDFLAGS)

mac:
	$(CC) $(CFLAGS) -c $(COMMON_C)
	$(CC) $(CFLAGS) $(COMMON_O) -o $(OUT) $(LDFLAGS)

