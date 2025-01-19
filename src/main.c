/*Copyright (c) 2025 Tristan Wellman*/
#include "engine/renderer.h"

void draw() {
	drawObject(getObjectFromName("Cube"));
}

void event() {
	SDL_Event event = getEvent();
	float camSpeed = 5.0f*getFrameTime();
	if(isKeyPressed()) {
		Vec3 pos = getCamPos();
		switch(getKeySym()) {
			case SDLK_w: moveCam(FRONT, camSpeed); break;
			case SDLK_s: moveCam(BACKWARD, camSpeed); break;
			case SDLK_a: moveCam(LEFT, camSpeed); break;
			case SDLK_d: moveCam(RIGHT, camSpeed); break;
			case SDLK_SPACE: moveCam(UP, camSpeed); break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT: moveCam(DOWN, camSpeed); break;
		};
	}	
}

void colorTest() {
	/*This is so you can convert a 255 color value to 1*/
	Color test = {100.0f, 181.0f, 72.0f, 255.0f};
	Color res = RGBA255TORGBA1(test);
	printf("(%f, %f, %f, %f)\n", res.r, res.g, res.b, res.a);
}

int main(int argc, char **argv) {
	
	initRenderer(1280, 720, "game", PERSPECTIVE);
	enableDebugInfo();
	
	while(rendererIsRunning()) {
		pollEvents((EVENTFUNC)event);
		renderFrame((RENDFUNC)draw);
	}
	
	return 0;
}
