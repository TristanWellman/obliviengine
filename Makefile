CC= gcc
CFLAGS= -g -O2  -Iinclude -Iinclude/SDL/include -Iinclude/assimp/include
CFLAGS_DEB= -g -O2 -Iinclude -Ishaders 
# We do not use LDFLAGS, but it is left here for lib reference.
LDFLAGS = -lm -lpthread 
LIB?= lib/libOE.a
UNAME_S := $(shell uname -s)
ARCH := $(shell $(CC) -dumpmachine)
SHDC= sokol-tools-bin/bin/win32/sokol-shdc.exe

# CIMGUI
CIMGUI_CC= g++
CIMGUI_SRCS := $(wildcard include/cimgui/*.cpp) \
			   $(wildcard include/cimgui/imgui/*.cpp) \
			   include/cimgui/imgui/backends/imgui_impl_sdl2.cpp \
			   include/cimgui/imgui/backends/imgui_impl_opengl3.cpp

CIMGUI_OBJS := $(CIMGUI_SRCS:.cpp=.o)
CIMGUI_CXXFLAGS = -std=c++11 -O2 -fno-exceptions -fno-rtti -Wall \
				  -DIMGUI_IMPL_OPENGL_LOADER_GL3W -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1 \
				  -DCIMGUI_USE_SDL2 -DCIMGUI_USE_OPENGL3 -DIMGUI_IMPL_API='extern "C"' \
				  -Iinclude/SDL/include -Iinclude/cimgui/imgui \
				  -Iinclude/cimgui -Iinclude/cimgui/imgui/backends
CIMGUI_LOC = lib/libcimgui.a

ifeq ($(UNAME_S),Linux)
	BACKEND= -DSOKOL_GLCORE
	LIB= lib/lin/libOE.a
	LDFLAGS += -ldl -lSDL2 -lcimgui -lassimp -llua -lX11 -lGL
	SHDC= sokol-tools-bin/bin/linux/sokol-shdc 
	CIMGUI_LDFLAGS += -ldl -lSDL2 -lcimgui -lX11 -lGL
	CIMGUI_LOC = lib/lin/libcimgui.a
endif
ifeq ($(UNAME_S),Darwin)
	BACKEND= -DSOKOL_GLCORE
	LIB= lib/mac/libOE.a
	LDFLAGS += -Llib/mac -ldl -lSDL2 -lcimgui -lassimp -llua -framework Cocoa -framework OpenGL 
	SHDC= sokol-tools-bin/bin/osx/sokol-shdc 
	CIMGUI_LDFLAGS += -Llib/mac -ldl -lSDL2 -framework Cocoa -framework OpenGL 
	CIMGUI_LOC = lib/mac/libcimgui.a
endif

ifeq ($(OS),Windows_NT)
	ifeq ($(ARCH),i686-w64-mingw32)
		LIB= lib/win32/libOE.a
		LDFLAGS += -Llib/win32 -lSDL2 -lcimgui -lassimp -lgdi32 -lopengl32
		CIMGUI_LDFLAGS += -Llib/win32 -lSDL2 -lgdi32 -lopengl32
		CIMGUI_LOC = lib/win32/libcimgui.a
	else
		LDFLAGS += -Llib -lSDL2 -lcimgui -lassimp -llua -lgdi32 -lopengl32
		CIMGUI_LDFLAGS += -Llib -lSDL2 -lgdi32 -lopengl32
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

.PHONY: test lib clean shaders shaders_clean cimgui clean_cimgui

all: shaders cimgui lib

.cpp.o:
	$(CIMGUI_CC) $(CIMGUI_CXXFLAGS) -c -o $@ $<

$(CIMGUI_LOC):$(CIMGUI_OBJS)
	$(AR) rcs $@ $(CIMGUI_OBJS)

cimgui: $(CIMGUI_LOC)

%.o: %.c
	$(CC) $(CFLAGS) $(BACKEND) -c $< -o $@

lib: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

test:
	$(MAKE) -C $(TEST_SRC)

clean: shaders_clean
	rm $(LIB) $(OBJS)

clean_cimgui:
	rm $(CIMGUI_OBJS) $(CIMGUI_LOC)

shaders:
	$(SHDC) --input shaders/simple.glsl --output include/OE/simple.glsl.h $(SHADER_ARGS) 
	$(SHDC) --input shaders/quad.glsl --output include/OE/quad.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/rayTracer.glsl --output include/OE/rayTracer.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/fxaa.glsl --output include/OE/fxaa.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/bloom.glsl --output include/OE/bloom.glsl.h $(SHADER_ARGS)
	$(SHDC) --input shaders/ssao.glsl --output include/OE/ssao.glsl.h $(SHADER_ARGS)

shaders_clean:
	rm include/OE/*.glsl.h
