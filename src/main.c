/*Copyright (c) 2025 Tristan Wellman
 *
 * This is a super simple example/testing program
 *
 * */
#include "engine/renderer.h"
//#include "engine/openxr/renderer_openxr.h"

void draw() {
	drawObject(getObjectFromName("Sphere"));
	//drawObject(getObjectFromName("OECube"));
	drawObject(getObjectFromName("OEPlane"));
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

void meshTest() {
	OEMesh mesh;
	OEParseObj("assets/models/sphere.obj", &mesh);
	sg_shader defShader = getDefCubeShader();
	OECreateObjectFromMesh(&mesh, (vec3){0.0f,0.0f,0.0f});
}

int main(int argc, char **argv) {
	
	initRenderer(1280, 720, "game", PERSPECTIVE);
	//initOpenXR();
	enableDebugInfo();
	meshTest();

	setObjectPosition("OEPlane", (vec3){0.0f, -1.0f, 0.0f});

	Color c = (Color){77.0f, 106.0f, 148.0f, 255.0f};
	OEAddLight("Test", (vec3){2.0f, 2.0f, 2.0f}, RGBA255TORGBA1(c));

	while(rendererIsRunning()) {
		pollEvents((EVENTFUNC)event);
		//pollXREvents();
		//renderXRFrame((RENDFUNC)draw);
		renderFrame((RENDFUNC)draw);
	}
	
	return 0;
}
