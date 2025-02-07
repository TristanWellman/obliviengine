CC= gcc
CFLAGS= -g -O2 -Iinclude -Iinclude/SDL/include
CFLAGS_DEB= -g -O2 -Iinclude -Ishaders 
LDFLAGS = -lm -ldl -lpthread 
UNAME_S := $(shell uname -s)
SHADER= shaders_win
ifeq ($(UNAME_S),Linux)
	BACKEND= -DSOKOL_GLCORE
    LDFLAGS += -Llib -lSDL2 -lX11 -lGL
endif
ifeq ($(UNAME_S),Darwin)
	BACKEND = -DSOKOL_GLCORE
    LDFLAGS += -Llib/mac -lSDL2 -framework Cocoa -framework OpenGL 
	SHADER = shaders_mac
endif
ifeq ($(OS),Windows_NT)
	#BACKEND = -DSOKOL_D3D11
	BACKEND = -DSOKOL_GLCORE

    LDFLAGS += -Llib -lSDL2 -lgdi32 -lopengl32
endif

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

# Check for OpenXR (Windows)
OPENXRINC= C:/msys64/mingw64/include/openxr
OPENXRLIB= C:/msys64/mingw64/lib
ifneq ($(wildcard C:/msys64/mingw64/include/openxr*),)
	CFLAGS += -I$(OPENXRINC) #-DSDL_VIDEO_DRIVER_WINDOWS
	CFLAGS_DEB += -I$(OPENXRINC) #-DSDL_VIDEO_DRIVER_WINDOWS
	LDFLAGS += -L$(OPENXRLIB) -lopenxr_loader.dll
	SRCS += $(wildcard src/openxr/*.c)
	OBJS = $(SRCS:.c=.o)
endif

LIB= lib/libOE.a
AR_ARGS= rcs $(LIB) $(COMMON_O)

SHDC_WIN= sokol-tools-bin/bin/win32/sokol-shdc.exe 
SHDC_MAC= sokol-tools-bin/bin/osx/sokol-shdc 
SHADER_ARGS= --output include/OE/simple.glsl.h --format sokol --slang glsl410

TEST_SRC= test

.PHONY: test lib clean shaders_win shaders_mac shaders_clean

all: $(SHADER) lib

%.o: %.c
	$(CC) $(CFLAGS) $(BACKEND) -c $< -o $@

lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

test:
	$(MAKE) -C $(TEST_SRC)

clean: shaders_clean
	rm $(OBJS) 

shaders_win:
	$(SHDC_WIN) --input shaders/simple.glsl $(SHADER_ARGS) 

shaders_mac:
	$(SHDC_MAC) --input shaders/simple.glsl $(SHADER_ARGS)

shaders_clean:
	rm include/OE/*glsl.h
