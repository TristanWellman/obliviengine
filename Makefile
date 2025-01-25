CC= gcc
CFLAGS= -O2 -Iinclude -Ishaders 
CFLAGS_DEB= -g -O2 -Iinclude -Ishaders 
LDFLAGS = -lm -ldl -lpthread 
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	BACKEND= -DSOKOL_GLCORE
    LDFLAGS += -Llib -lSDL2 -lX11 -lGL
endif
ifeq ($(UNAME_S),Darwin)
	BACKEND = -DSOKOL_GLCORE
    LDFLAGS += -Llib/mac -lSDL2 -framework Cocoa -framework OpenGL 
endif
ifeq ($(OS),Windows_NT)
	#BACKEND = -DSOKOL_D3D11
	BACKEND = -DSOKOL_GLCORE

    LDFLAGS += -Llib -lSDL2 -lgdi32 -lopengl32
endif

COMMON_C= src/*.c \
		  src/engine/*.c

COMMON_O= *.o

# Check for OpenXR (Windows)
OPENXRINC= C:/msys64/mingw64/include/openxr
OPENXRLIB= C:/msys64/mingw64/lib
ifneq ($(wildcard C:/msys64/mingw64/include/openxr*),)
	CFLAGS += -I$(OPENXRINC) #-DSDL_VIDEO_DRIVER_WINDOWS
	CFLAGS_DEB += -I$(OPENXRINC) #-DSDL_VIDEO_DRIVER_WINDOWS
	LDFLAGS += -L$(OPENXRLIB) -lopenxr_loader.dll
	COMMON_C +=  src/engine/openxr/*.c
endif

BIN= game

all: shaders_clean shaders_win debug

release:
	$(CC) $(CFLAGS) $(BACKEND) -c $(COMMON_C)
	$(CC) $(CFLAGS) $(BACKEND) $(COMMON_O) -o $(BIN) $(LDFLAGS)

debug:
	$(CC) $(CFLAGS_DEB) $(BACKEND) -c $(COMMON_C)
	$(CC) $(CFLAGS_DEB) $(BACKEND) $(COMMON_O) -o $(BIN) $(LDFLAGS)

shaders_win:
	sokol-tools-bin/bin/win32/sokol-shdc.exe --input shaders/simple.glsl --output shaders/simple.glsl.h --format sokol --slang glsl410
	sokol-tools-bin/bin/win32/sokol-shdc --input shaders/light.glsl --output shaders/light.glsl.h --format sokol --slang glsl410

shaders_mac:
	sokol-tools-bin/bin/osx/sokol-shdc --input shaders/simple.glsl --output shaders/simple.glsl.h --format sokol --slang glsl410
	sokol-tools-bin/bin/osx/sokol-shdc --input shaders/light.glsl --output shaders/light.glsl.h --format sokol --slang glsl410

shaders_clean:
	rm shaders/*.h
