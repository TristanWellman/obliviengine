CC= gcc
CFLAGS= -g -O2 -Iinclude -Iinclude/SDL/include
CFLAGS_DEB= -g -O2 -Iinclude -Ishaders 
LDFLAGS = -lm -lpthread 
LIB?= lib/libOE.a
UNAME_S := $(shell uname -s)
ARCH := $(shell $(CC) -dumpmachine)
SHDC= sokol-tools-bin/bin/win32/sokol-shdc.exe
ifeq ($(UNAME_S),Linux)
	BACKEND= -DSOKOL_GLCORE
	LIB= lib/lin/libOE.a
    LDFLAGS += -ldl -lSDL2 -lX11 -lGL
	SHDC= sokol-tools-bin/bin/linux/sokol-shdc 
endif
ifeq ($(UNAME_S),Darwin)
	BACKEND= -DSOKOL_GLCORE
	LIB= lib/mac/libOE.a
    LDFLAGS += -Llib/mac -ldl -lSDL2 -framework Cocoa -framework OpenGL 
	SHDC= sokol-tools-bin/bin/osx/sokol-shdc 
endif

ifeq ($(OS),Windows_NT)
	ifeq ($(ARCH),i686-w64-mingw32)
		LIB= lib/win32/libOE.a
		LDFLAGS += -Llib/win32 -lSDL2 -lgdi32 -lopengl32
	else
		LDFLAGS += -Llib -lSDL2 -lgdi32 -lopengl32
	endif
	BACKEND= -DSOKOL_GLCORE
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

AR_ARGS= rcs $(LIB) $(COMMON_O)

SHADER_ARGS=  --format sokol --slang glsl410 --ifdef 

TEST_SRC= test

.PHONY: test lib clean shaders shaders_clean

all: shaders lib

%.o: %.c
	$(CC) $(CFLAGS) $(BACKEND) -c $< -o $@

lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

test:
	$(MAKE) -C $(TEST_SRC)

clean: shaders_clean
	rm $(OBJS) 

shaders:
	$(SHDC) --input shaders/simple.glsl --output include/OE/simple.glsl.h $(SHADER_ARGS) 
	$(SHDC) --input shaders/quad.glsl --output include/OE/quad.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/rayTracer.glsl --output include/OE/rayTracer.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/fxaa.glsl --output include/OE/fxaa.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/bloom.glsl --output include/OE/bloom.glsl.h $(SHADER_ARGS)

shaders_clean:
	rm include/OE/*.glsl.h
