/*Copyright (c) 2025 Tristan Wellman
 *
 * This is a super simple example/testing program
 *
 * */
#include <OE/OE.h>
//#include "engine/openxr/OEOpenxr.h"

void draw() {
	//OEDrawObject(OEGetObjectFromName("Icosphere"));
	OEDrawObject(OEGetObjectFromName("Cube"));
	OEDrawObject(OEGetObjectFromName("OECube"));
	OEDrawObject(OEGetObjectFromName("OEPlane"));
}

void event() {
	SDL_Event event = OEGetEvent();
	float camSpeed = 5.0f*OEGetFrameTime();
	if(OEIsKeyPressed()) {
		Vec3 pos = OEGetCamPos();
		switch(OEGetKeySym()) {
			case SDLK_w: OEMoveCam(FRONT, camSpeed); break;
			case SDLK_s: OEMoveCam(BACKWARD, camSpeed); break;
			case SDLK_a: OEMoveCam(LEFT, camSpeed); break;
			case SDLK_d: OEMoveCam(RIGHT, camSpeed); break;
			case SDLK_SPACE: OEMoveCam(UP, camSpeed); break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT: OEMoveCam(DOWN, camSpeed); break;
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
	OEMesh mesh, sphere;
	OEParseObj("assets/models/cube.obj", &mesh);
	sg_shader defShader = OEGetDefCubeShader();
	OECreateObjectFromMesh(&mesh, (vec3){0.0f,0.0f,0.0f});
	/*OEParseObj("assets/models/sphere.obj", &sphere);
	OECreateObjectFromMesh(&sphere, (vec3){3.0f,0.0f,0.0f});*/

}

int main(int argc, char **argv) {
	
	OEInitRenderer(1280, 720, "game", PERSPECTIVE);
	//initOpenXR();
	OEEnableDebugInfo();
	meshTest();

	OESetObjectPosition("OEPlane", (vec3){0.0f, -1.0f, 0.0f});
	OESetObjectPosition("OECube", (vec3){-3.0f, 0.0f, 0.0f});

	Color c = (Color){77.0f, 106.0f, 148.0f, 255.0f};
	OEAddLight("Test", (vec3){2.0f, 2.0f, 2.0f}, RGBA255TORGBA1(c));

	//addTexture("assets/stone.png", "test");

	while(OERendererIsRunning()) {
		OEPollEvents((EVENTFUNC)event);
		//OEPollXREvents();
		//OERenderXRFrame((RENDFUNC)draw);
		OERenderFrame((RENDFUNC)draw);
	}
	
	return 0;
}
