CC= gcc
CFLAGS= -g -O2 -I../../include -I../../include/SDL/include
LDFLAGS= -lm -ldl -lpthread

LDFLAGS_MAC= $(LDFLAGS) -L../../lib/mac -L../../lib -lOE -lSDL2 -framework Cocoa -framework OpenGL
LDFLAGS_WIN= $(LDFLAGS) -L../../lib -lOE -lSDL2 -lgdi32 -lopengl32

CUBE= main.c

OUT= cube

.PHONY: build_win build_mac

all: build

build_win:
	$(CC) $(CFLAGS) $(CUBE) -o $(OUT) $(LDFLAGS_WIN)

build_mac:
	$(CC) $(CFLAGS) $(CUBE) -o $(OUT) $(LDFLAGS_MAC)

