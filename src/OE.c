/*Copyright (c) 2025 Tristan Wellman*/
#define SOKOL_IMPL
#include <OE/OE.h>
#include <OE/cube.h>

//#include <simple.glsl.h>

Object *OEGetObjectFromName(char *name) {
	if(name==NULL) return NULL;
	int i;
	for(i=0;i<globalRenderer->objSize;i++) {
		if(globalRenderer->objects[i].name==NULL) continue;
		if(!strcmp(globalRenderer->objects[i].name, name))
			return &globalRenderer->objects[i];
	}
	return NULL;
}

void OECreateObject(Object obj) {
	if(obj.name==NULL) return;
	int i;
	/*Check for dups*/
	for(i=0;i<globalRenderer->objSize;i++) {
		if(!strcmp(obj.name, globalRenderer->objects[i].name)) {
			char buf[strlen(obj.name)+128];
			sprintf(buf, "Duplicate object names: %s. Not adding.", obj.name);
			WLOG(WARN, buf);
			return;
		}
	}
	if(globalRenderer->objSize>=globalRenderer->objCap) {
		globalRenderer->objCap*=2;
		globalRenderer->objects =
			(Object *)realloc(globalRenderer->objects, 
					sizeof(Object)*globalRenderer->objCap);
	}
	for(i=0;i<globalRenderer->objSize&&globalRenderer->objects[i].name!=NULL;i++);
	size_t pos = globalRenderer->objSize;
	memcpy(&globalRenderer->objects[pos].pipe, &obj.pipe, sizeof(obj.pipe));
	memcpy(&globalRenderer->objects[pos].vbuf, &obj.vbuf, sizeof(obj.vbuf));
	memcpy(&globalRenderer->objects[pos].ibuf, &obj.ibuf, sizeof(obj.ibuf));
	memcpy(&globalRenderer->objects[pos].defShader, &obj.defShader, sizeof(obj.defShader));
	
	globalRenderer->objects[pos].name = calloc(strlen(obj.name)+1, sizeof(char));
	strcpy(globalRenderer->objects[pos].name, obj.name);
	if(obj.ID!=NULL) {
		globalRenderer->objects[pos].ID = calloc(strlen(obj.ID)+1, sizeof(char));
		strcpy(globalRenderer->objects[pos].ID, obj.ID);
	}

	globalRenderer->objects[pos].numIndices = obj.numIndices;
	mat4x4_dup(globalRenderer->objects[pos].model,  obj.model);
	mat4x4_dup(globalRenderer->objects[pos].originalModel, obj.model);
	vec3_dup(globalRenderer->objects[pos].pos, obj.pos);

	globalRenderer->objects[pos].numID = globalRenderer->objSize;
	globalRenderer->objSize++;

	char buf[strlen(globalRenderer->objects[pos].name)+256];
	sprintf(buf, "OBJ[%zu]: %s Created", pos, globalRenderer->objects[pos].name);
	WLOG(INFO, buf);

}

void OECreateObjectEx(char *name, vec3 pos,
		sg_buffer_desc vbuf, sg_buffer_desc ibuf, sg_shader defShader,
		sg_pipeline_desc pipe) {
	Object obj = {0};
	obj.name = calloc(strlen(name)+1, sizeof(char));
	strcpy(obj.name, name);

	obj.numIndices = ibuf.data.size / sizeof(uint16_t);

	obj.vbuf = sg_make_buffer(&vbuf);
	obj.ibuf= sg_make_buffer(&ibuf);
	memcpy(&obj.defShader, &defShader, sizeof(defShader));
	obj.pipe = sg_make_pipeline(&pipe);

	mat4x4_identity(obj.model);

	obj.pos[0] = pos[0];
	obj.pos[1] = pos[1];
	obj.pos[2] = pos[2];
	mat4x4_identity(obj.model);
	mat4x4_translate(obj.model, obj.pos[0], obj.pos[1], obj.pos[2]);
	mat4x4_dup(obj.originalModel, obj.model);

	OECreateObject(obj);
	free(obj.name);
}

void OECreateObjectFromMesh(OEMesh *mesh, vec3 pos
		/*sg_shader defShader, sg_pipeline_desc pipe*/) {
	if(mesh==NULL) return;
	Object obj = {0};
	obj.name = calloc(strlen(mesh->label)+1, sizeof(char));
	strcpy(obj.name, mesh->label);

	obj.numIndices = mesh->indices.size*6;

	/*Expand mesh verts and indices*/
	/*4+VSIZE cuz 4 color values, 3 normal values, 2 tex coords*/
	int vertSize = mesh->verts.total+(mesh->verts.size*(6+VSIZE));
	int indSize = mesh->indices.size*6;
	float *finalVerts = calloc(vertSize, sizeof(float));
	uint16_t *finalInds = calloc(indSize, sizeof(uint16_t));
	/*This REQUIRES 3 points per vert, 
	 * if there isn't you've done something wrong and it'll crash*/
	int i,j=0, l,k;
	for(i=0;i<vertSize;i+=(VSIZE+(6+VSIZE)),j++) {
		/*verts*/
		finalVerts[i] = mesh->verts.data[j][0];
		finalVerts[i+1] = mesh->verts.data[j][1];
		finalVerts[i+2] = mesh->verts.data[j][2];

		/*color*/
		finalVerts[i+3] = 1.0f;
		finalVerts[i+4] = 1.0f;
		finalVerts[i+5] = 1.0f;
		finalVerts[i+6] = 1.0f;

		/*Normals*/
		/*TODO Use faster lookup method!*/
    	vec3 normal = {0.0f, 0.0f, 0.0f};

		for(l=0;l<mesh->normInds.size;l++) {
			for(k=0;k<ISIZE;k++) {
				if(mesh->indices.data[l][k]-1==j) {
					int pos = mesh->normInds.data[l][k] - 1;
					normal[0] += mesh->vertNorms.data[pos][0];
					normal[1] += mesh->vertNorms.data[pos][1];
					normal[2] += mesh->vertNorms.data[pos][2];	
				}
			}
		}
		
		Vec3 tmp = {normal[0],normal[1],normal[2]};
		tmp = WNORM(tmp);
		VEC3TOVEC3F(tmp, normal);

		finalVerts[i+7] = normal[0];
		finalVerts[i+8] = normal[1];
		finalVerts[i+9] = normal[2];

		/*Tex Coords*/
		vec2 texCoords = {0.0f, 0.0f};

		for(l=0;l<mesh->texInds.size;l++) {
			for(k=0;k<ISIZE;k++) {
				if(mesh->indices.data[l][k]-1==j) {
					int pos = mesh->texInds.data[l][k] - 1;
					texCoords[0] += mesh->vertTex.data[pos][0];
					texCoords[1] += mesh->vertTex.data[pos][1];
				}
			}
		}

		finalVerts[i+10] = texCoords[0];
		finalVerts[i+11] = texCoords[1];

	}
	/*This REQUIRES 4 points per face,and atleast 6 faces.
	 * If there isn't you've done something wrong and it'll crash*/
	for(i=0,j=0;i<indSize;i+=6,j++) {
    	finalInds[i]   = mesh->indices.data[j][0] - 1;
    	finalInds[i+1] = mesh->indices.data[j][2] - 1;
		finalInds[i+2] = mesh->indices.data[j][1] - 1;
	    finalInds[i+3] = mesh->indices.data[j][0] - 1;
	    finalInds[i+4] = mesh->indices.data[j][3] - 1;
	    finalInds[i+5] = mesh->indices.data[j][2] - 1;
	}
	

	obj.vbuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = PTRRANGE(finalVerts, vertSize),
				.label = "objv"
			});
	obj.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.usage.index_buffer = true,
				.data = PTRRANGE(finalInds, indSize),
				.label = "obji"
			});

	sg_shader defShader = OEGetDefCubeShader();
	memcpy(&obj.defShader, &defShader, sizeof(defShader));
	sg_pipeline_desc pipe = OEGetDefaultPipe(defShader, "mesh");
	obj.pipe = sg_make_pipeline(&pipe);

	obj.pos[0] = pos[0];
	obj.pos[1] = pos[1];
	obj.pos[2] = pos[2];
	mat4x4_identity(obj.model);
	mat4x4_translate(obj.model, obj.pos[0], obj.pos[1], obj.pos[2]);
	mat4x4_dup(obj.originalModel, obj.model);

	OECreateObject(obj);
	free(obj.name);
	free(finalVerts);
	free(finalInds);
}

void createObjPipe() {

}

void OESetObjectPosition(char *ID, vec3 position) {
	Object *obj = OEGetObjectFromName(ID);
	if(obj!=NULL) {
		obj->pos[0] = position[0];
		obj->pos[1] = position[1]; 
		obj->pos[2] = position[2]; 
	
		mat4x4_identity(obj->model);
		mat4x4_translate(obj->model, obj->pos[0], obj->pos[1], obj->pos[2]);
	}
}

void OERotateObject(char *ID, float deg) {
	Object *obj = OEGetObjectFromName(ID);
	if(obj!=NULL) {
		float rad = DEG2RAD(deg);
		mat4x4 rot, trans;
		mat4x4_identity(rot);
		mat4x4_identity(trans);
		mat4x4_rotate(rot, rot, 0.0f, 1.0f, 0.0f ,rad);
		mat4x4_translate(trans, obj->pos[0], obj->pos[1], obj->pos[2]);
		mat4x4_mul(obj->model, trans, rot);
	}
}

void OEResetRotation(char *ID) {
	Object *obj = OEGetObjectFromName(ID);
	if(obj!=NULL) mat4x4_dup(obj->model, obj->originalModel);
}

void setObjectShader(char *name, sg_shader shd) {
	Object *obj = OEGetObjectFromName(name);
	if(obj!=NULL) obj->defShader = shd;
}

/*This is specifically for the default shader*/
void OEApplyCurrentUniforms(Object *obj) {
    mat4x4 mvp, mv;
    mat4x4_mul(mv, globalRenderer->cam.view, obj->model);
    mat4x4_mul(mvp, globalRenderer->cam.proj, mv);

	vs_params_t vs_params;
	fs_params_t fs_params;
	light_params_t light_params = getLightUniform();
	Camera *cam = OEGetCamera();
    memcpy(vs_params.mvp, mvp, sizeof(mvp));
	memcpy(vs_params.model, obj->model, sizeof(obj->model));
	memcpy(fs_params.camPos, cam->position, sizeof(cam->position));

    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
	sg_apply_uniforms(UB_fs_params, &SG_RANGE(fs_params));
	sg_apply_uniforms(UB_light_params, &SG_RANGE(light_params));
}

void *applyFXAAUniforms() {
	OEFXAA_resolution_t params;
	vec2_dup(params.resolution, (vec2){
		globalRenderer->window->width,
		globalRenderer->window->height});
	sg_apply_uniforms(UB_OEFXAA_resolution, &SG_RANGE(params));
	return NULL;
}

void *applyBloomUniforms() {
	sg_apply_uniforms(UB_OEBloom_params, &SG_RANGE(globalRenderer->bloomParams));
	return NULL;
}

/*void *applySSAOUniforms() {
	sg_apply_uniforms(UB_OESSAO_params, &SG_RANGE(globalRenderer->ssao.params));
	return NULL;
}*/

void OEDrawObject(Object *obj) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.images[OE_TEXPOS] = OEGetDefaultTexture(),
        .samplers[OE_TEXPOS] = globalRenderer->ssao.sampler

    });

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectBind(Object *obj, sg_bindings binding) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&binding);

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}


void OEDrawObjectTex(Object *obj, int assign, sg_image texture) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}
	const int a = 3;
    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.images[a] = texture,
        .samplers[a] = globalRenderer->ssao.sampler

    });

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectEx(Object *obj, UNILOADER apply_uniforms) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}
	
    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.images[OE_TEXPOS] = OEGetDefaultTexture(),
        .samplers[OE_TEXPOS] = globalRenderer->ssao.sampler
    });

	apply_uniforms();

    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectTexEx(Object *obj, int assign,
		sg_image texture, UNILOADER apply_uniforms) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}
	const int a = 3;	
    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.images[a] = texture,
        .samplers[a] = globalRenderer->ssao.sampler
    });

	apply_uniforms();

    sg_draw(0, obj->numIndices, 1);
}

int OERendererIsRunning() {
	return globalRenderer->window->running;
}

sg_buffer_desc OEGetCubeVertDesc() {
	return (sg_buffer_desc) {
				.data = SG_RANGE(cubeVertices),
				.label = "cube_verts"
			};
}

sg_buffer_desc OEGetCubeIndDesc() {
	return (sg_buffer_desc){
				.usage.index_buffer = true,
				.data = SG_RANGE(cubeIndices),
				.label = "cube_indices"
			};
}

sg_environment OEGetEnv(void) {
	return (sg_environment) {
		.defaults = {
			.color_format = SG_PIXELFORMAT_RGBA8,
			.depth_format = SG_PIXELFORMAT_NONE,
			.sample_count = 1,
		},
	};
}

sg_swapchain OEGetSwapChain(void) {
	int w = globalRenderer->window->width;
	int h = globalRenderer->window->height;
	return (sg_swapchain) {
		.sample_count = 1,
		.color_format = SG_PIXELFORMAT_RGBA8,
		.depth_format = SG_PIXELFORMAT_NONE,
		.width = w,
		.height = h,
		.gl.framebuffer = 0,
	};
}

sg_pipeline_desc OEGetDefaultPipe(sg_shader shader, char *label) {
	char *_label = calloc(strlen(label)+1, sizeof(char));
	strcpy(_label, label);
	return (sg_pipeline_desc){
			.shader = shader,
			.layout = {
            	.attrs = {
                	[ATTR_simple_position].format = SG_VERTEXFORMAT_FLOAT3,
					[ATTR_simple_color0].format = SG_VERTEXFORMAT_FLOAT4,
					[ATTR_simple_normal0].format = SG_VERTEXFORMAT_FLOAT3,
					[ATTR_simple_texcoord0].format = SG_VERTEXFORMAT_FLOAT2
            	}
        	},
			.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
			.index_type = SG_INDEXTYPE_UINT16,
        	.cull_mode = SG_CULLMODE_BACK,
			.sample_count = 1,
			.color_count = 4,
			.colors = {
				[0] = {.pixel_format = SG_PIXELFORMAT_RGBA8}, /*Color Buffer*/
				[1] = {.pixel_format = SG_PIXELFORMAT_RGBA8}, /*Depth Buffer*/
				[2] = {.pixel_format = SG_PIXELFORMAT_RGBA8}, /*Normal Buffer*/
				[3] = {.pixel_format = SG_PIXELFORMAT_RGBA8}, /*Position Buffer*/
			},
        	.depth = {
				.pixel_format = SG_PIXELFORMAT_NONE,
        	},
			.label = _label
		};
}

sg_pipeline_desc OEGetQuadPipeline(sg_shader shader, char *label) {
	char *_label = calloc(strlen(label)+1, sizeof(char));
	strcpy(_label, label);
	return (sg_pipeline_desc) {
		.shader = shader,
		.layout = {
			.attrs = {
				[ATTR_quad_OEquad_position].format = SG_VERTEXFORMAT_FLOAT2,
				[ATTR_quad_OEquad_texcoord].format = SG_VERTEXFORMAT_FLOAT2,
			}
		},
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
		.index_type = SG_INDEXTYPE_NONE,
		.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .depth = {
			.pixel_format = SG_PIXELFORMAT_NONE,
        },
		.label = _label
	};
}

sg_shader OEGetDefCubeShader() {
	return globalRenderer->defCubeShader;
}

sg_shader OEGetRayTracedShader() {
	if(globalRenderer->rayTracedShader.id==SG_INVALID_ID)
		globalRenderer->rayTracedShader = sg_make_shader(OERayTracer_shader_desc(sg_query_backend()));
		
	return globalRenderer->rayTracedShader;
}

sg_pipeline_desc OEGetRayTracedPipe() {
	return OEGetDefaultPipe(OEGetRayTracedShader(), "rayTracedPipe");
}


void OESetDefaultShader(sg_shader shader) {
	globalRenderer->defCubeShader = shader;
	int i;
	for(i=0;i<globalRenderer->objSize&&
			globalRenderer->objects[i].name!=NULL;i++) {
		globalRenderer->objects[i].defShader = shader;
		sg_destroy_pipeline(globalRenderer->objects[i].pipe);
		sg_pipeline_desc pipe = OEGetDefaultPipe(shader, globalRenderer->objects[i].name);
		globalRenderer->objects[i].pipe = sg_make_pipeline(&pipe);
	}
}

Object OEGetDefaultCubeObj(char *name) {
	Object obj = {0};
	obj.name = calloc(strlen(name)+1, sizeof(char));
	strcpy(obj.name, name);
	obj.vbuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(cubeVertices),
				.label = "cube_verts"
			});
	obj.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.usage.index_buffer = true,
				.data = SG_RANGE(cubeIndices),
				.label = "cube_indices"
			});
	obj.numIndices = ARRLEN(cubeIndices);
	obj.defShader = globalRenderer->defCubeShader;
	sg_pipeline_desc pipe = OEGetDefaultPipe(obj.defShader, obj.name);
	obj.pipe = sg_make_pipeline(&pipe);
	mat4x4_identity(obj.model);
	return obj;
}

sg_image OEGetDefaultTexture() {
	return globalRenderer->defTexture;
}

void initBaseObjects() {

	/*Create a test light*/
	Color c = (Color){255.0f, 135.0f, 102.0f, 255.0f};
	OEAddLight("Light1", (vec3){-5.0f, 5.0f, -5.0f},
			RGBA255TORGBA1(c));

	Object test = {0};
	test.vbuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(cubeVertices),
				.label = "cube_verts"
			});
	WASSERT(test.vbuf.id!=SG_INVALID_ID,
			"ERROR:: Failed to init vbuf!");

	test.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.usage.index_buffer = true,
				.data = SG_RANGE(cubeIndices),
				.label = "cube_indices"
			});

	test.numIndices = ARRLEN(cubeIndices);
	mat4x4_identity(test.model);

	test.defShader = globalRenderer->defCubeShader; 

	test.name = "OECube";	

	sg_pipeline_desc cube_pipe = OEGetDefaultPipe(test.defShader, test.name);
	test.pipe = sg_make_pipeline(&cube_pipe);


	OECreateObject(test);

	Object plane = {0};

	plane.vbuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(planeVertices),
				.label = "plane_verts"
			});
	WASSERT(plane.vbuf.id!=SG_INVALID_ID,
			"ERROR:: Failed to init vbuf!");

	
	plane.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.usage.index_buffer = true,
				.data = SG_RANGE(planeIndices),
				.label = "plane_indices"
			});

	plane.numIndices = ARRLEN(planeIndices);
	mat4x4_identity(plane.model);

	plane.name = "OEPlane";
	plane.defShader = globalRenderer->defCubeShader;
	sg_pipeline_desc plane_pipe = OEGetDefaultPipe(plane.defShader, plane.name);
	plane.pipe = sg_make_pipeline(&plane_pipe);

	OECreateObject(plane);
	
}

void OEEnableDebugInfo() {
	globalRenderer->debug = 1;
}

void computeCameraRay() {
	Camera *cam = OEGetCamera();
	vec3 ray_origin, ray_dir;
	vec3_dup(ray_origin, cam->position);
	vec3_dup(ray_dir, cam->front);
	float ground = -ray_origin[1]/ray_dir[1];
	vec3_scale(cam->ray_hit, ray_dir, ground);
	vec3_add(cam->ray_hit, cam->ray_hit, ray_origin);
}

void OEComputeRotationMatrix(mat4x4 out, vec3 front, vec3 up) {
	vec3 f,u,s;
	vec3_norm(f, front);
	vec3_norm(u, up);
	vec3_mul_cross(s, f, u);

	vec3_mul_cross(u, s, f);

	out[0][0] = s[0]; out[0][1] = s[1]; out[0][2] = s[2]; out[0][3] = 0.0f;
	out[1][0] = u[0]; out[1][1] = u[1]; out[1][2] = u[2]; out[1][3] = 0.0f;
	out[2][0] = -f[0]; out[2][1] = -f[1]; out[2][2] = -f[2]; out[2][3] = 0.0f;
	out[3][0] = 0.0f; out[3][1] = 0.0f; out[3][2] = 0.0f; out[3][3] = 1.0f;
}

void OEInitRenderer(int width, int height, char *title, enum CamType camType) {

/*
 * GlobalRenderer & OpenGL/SDL setup
 * */

	globalRenderer = calloc(1, sizeof(struct renderer));
	globalRenderer->window = calloc(1, sizeof(Window));
	globalRenderer->window->width = width;
	globalRenderer->window->height = height;
	globalRenderer->window->title = calloc(strlen(title)+1, sizeof(char *));
	strcpy(globalRenderer->window->title, title);
	globalRenderer->window->running = 1;
	globalRenderer->debug = 0;
	globalRenderer->postPassSize = 0;

	WASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)>=0,
			"ERROR:: Failed to init SDL!");

	SDL_GL_LoadLibrary(NULL);
#if defined __APPLE__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
#endif
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	

	globalRenderer->window->window = SDL_CreateWindow(
			globalRenderer->window->title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			globalRenderer->window->width, globalRenderer->window->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	globalRenderer->window->gl_context = 
		SDL_GL_CreateContext(globalRenderer->window->window);
	WASSERT(globalRenderer->window->gl_context!=NULL,
			"ERROR:: Failed to init gl context");

	WASSERT(gladLoadGLLoader(SDL_GL_GetProcAddress),
			"ERROR:: Failed to initialize GLAD!");

	SDL_GL_SetSwapInterval(1);
 	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
	glFrontFace(GL_CCW);

	int w,h;
	SDL_GetWindowSize(globalRenderer->window->window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	WLOG(INFO_VENDOR, glGetString(GL_VENDOR));
	WLOG(INFO_GPU, glGetString(GL_RENDERER));
	WLOG(INFO_DRIVER_VERSION, glGetString(GL_VERSION));

/*
 * Sokol setup
 * */


	sg_setup(&(sg_desc){
			.environment = OEGetEnv(),
			.logger.func = slog_func});

    sdtx_setup(&(sdtx_desc_t){
        .fonts = {[1]  = sdtx_font_oric()},
        .logger.func = slog_func});

/*
 * setup Render Target & Depth Buffer
 * */

	globalRenderer->ssao.w = globalRenderer->window->width;
	globalRenderer->ssao.h = globalRenderer->window->height;

	globalRenderer->renderTarget = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA8,
		//.sample_count = 4,
		.label = "render_target"
	});

	globalRenderer->postTarget = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA8,
		.label = "post_target"
	});
	globalRenderer->postTargetPong = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA8,
		.label = "post_target_p"
	});

	globalRenderer->depthBuffer = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA8,
		.sample_count = 1,
		.label = "depth_image"
	});
	globalRenderer->normalBuffer = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA8,
		.sample_count = 1,
		.label = "normal_image"
	});
	globalRenderer->positionBuffer = sg_make_image(&(sg_image_desc){
		.usage.render_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA8,
		.sample_count = 1,
		.label = "position_image"
	});

	globalRenderer->renderTargetAtt = sg_make_attachments(&(sg_attachments_desc){
		.colors[0].image = globalRenderer->renderTarget,
		.colors[1].image = globalRenderer->depthBuffer,
		.colors[2].image = globalRenderer->normalBuffer,
		.colors[3].image = globalRenderer->positionBuffer,
		.label = "render_target_atts"});
	
	globalRenderer->postTargetAtt = sg_make_attachments(&(sg_attachments_desc){
		.colors[0].image = globalRenderer->postTarget,
		.label = "post_target_atts"});
	globalRenderer->postTargetAttPong = sg_make_attachments(&(sg_attachments_desc){
		.colors[0].image = globalRenderer->postTargetPong,
		.label = "post_target_atts_p"});

	/*the sampler is important*/
	globalRenderer->ssao.sampler = sg_make_sampler(&(sg_sampler_desc){
		.min_filter = SG_FILTER_NEAREST,
		.mag_filter = SG_FILTER_NEAREST,
		.wrap_u = SG_WRAP_REPEAT,           
		.wrap_v = SG_WRAP_REPEAT,
		.wrap_w = SG_WRAP_REPEAT,
		.label = "render_sampler"
    });

/*
 * Create the render targer shader and pipeline
 * */

	globalRenderer->renderTargetShade = sg_make_shader(quad_shader_desc(sg_query_backend()));

	sg_pipeline_desc rtp = OEGetQuadPipeline(globalRenderer->renderTargetShade, 
			"render_target_pipe");

	globalRenderer->renderTargetPipe = sg_make_pipeline(&rtp);

	globalRenderer->renderTargetBuff = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(quadVertices),
				.label = "quad_verts"
			});


/*
 * Init objects
 * */
	globalRenderer->objCap = MAXOBJS;
	globalRenderer->objSize = 0;
	globalRenderer->objects = calloc(globalRenderer->objCap, sizeof(Object));
	
	globalRenderer->defCubeShader = sg_make_shader(simple_shader_desc(sg_query_backend()));

	initBaseObjects();

	static const uint32_t white = OE_WHITEP;
	sg_image_desc img_desc = {
		.width = 1,
		.height = 1,
		.pixel_format = SG_PIXELFORMAT_RGBA8,
		.data.subimage[0][0] = {
			.ptr = &white,
			.size = sizeof(white)
		},
    	.label = "defTexture"
	};

	globalRenderer->defTexture = sg_make_image(&img_desc);

	/*Setup Camera*/

	globalRenderer->cam.oScale = 10.0f;
	float oScale = globalRenderer->cam.oScale;

	mat4x4_identity(globalRenderer->cam.model);
	mat4x4_identity(globalRenderer->cam.view);
	mat4x4_identity(globalRenderer->cam.proj);

	vec3_dup(globalRenderer->cam.up, (vec3){0.0f, 1.0f, 0.0f});
	globalRenderer->cam.fov = DEG2RAD(80.0f);

	float aspect = (float)globalRenderer->window->width / (float)globalRenderer->window->height;
	globalRenderer->cam.aspect = aspect;
	Vec3 tmp;

	globalRenderer->camType = camType;
	switch(camType) {
		case ISOMETRIC:
			//globalRenderer->cam.fov = 80.0f;
			vec3_dup(globalRenderer->cam.position, (vec3){15.0f, 15.0f, 15.0f});

			float angle_y = DEG2RAD(45.0f);
			float angle_x = DEG2RAD(-30.0f);
	
			globalRenderer->cam.front[0] = -1.0f; 
			globalRenderer->cam.front[1] = -1.0f;
			globalRenderer->cam.front[2] = -1.0f;
			tmp = (Vec3){globalRenderer->cam.front[0],globalRenderer->cam.front[1],globalRenderer->cam.front[2]};
			tmp = WNORM(tmp);
			VEC3TOVEC3F(tmp, globalRenderer->cam.front);

			vec3_mul_cross(globalRenderer->cam.right, globalRenderer->cam.front, globalRenderer->cam.up);
			tmp = (Vec3){globalRenderer->cam.right[0],globalRenderer->cam.right[1],globalRenderer->cam.right[2]};
			tmp = WNORM(tmp);
			VEC3TOVEC3F(tmp, globalRenderer->cam.right);
			vec3_mul_cross(globalRenderer->cam.up, globalRenderer->cam.right, globalRenderer->cam.front);

			//vec3_add(globalRenderer->cam.target, globalRenderer->cam.position, globalRenderer->cam.front);
			vec3_dup(globalRenderer->cam.target, (vec3){0.0f,0.0f,0.0f});
			mat4x4_look_at(globalRenderer->cam.view,
			               globalRenderer->cam.position,
			               globalRenderer->cam.target,
			               globalRenderer->cam.up);

			mat4x4_ortho(globalRenderer->cam.proj, 
			             -oScale * aspect, oScale * aspect, 
			             -oScale, oScale, 
			             0.1f, 100.0f);

			mat4x4 tmp_;
			mat4x4_mul(tmp_, globalRenderer->cam.proj, globalRenderer->cam.view);
			mat4x4_mul(globalRenderer->cam.mvp, tmp_, globalRenderer->cam.model);

			break;
		case PERSPECTIVE: 
			vec3_dup(globalRenderer->cam.position, (vec3){0.0f, 0.0f, 5.0f});
			vec3_dup(globalRenderer->cam.target, (vec3){0.0f, 0.0f, 0.0f});
			vec3_dup(globalRenderer->cam.front, (vec3){0.0f, 0.0f, -1.0f});
			tmp = (Vec3){globalRenderer->cam.front[0],globalRenderer->cam.front[1],globalRenderer->cam.front[2]};
			tmp = WNORM(tmp);
			VEC3TOVEC3F(tmp, globalRenderer->cam.front);
			
			vec3_mul_cross(globalRenderer->cam.right, globalRenderer->cam.front, globalRenderer->cam.up);
			tmp = (Vec3){globalRenderer->cam.right[0],globalRenderer->cam.right[1],globalRenderer->cam.right[2]};
			tmp = WNORM(tmp);
			VEC3TOVEC3F(tmp, globalRenderer->cam.right);
			vec3_mul_cross(globalRenderer->cam.up, globalRenderer->cam.right, globalRenderer->cam.front);
			tmp = (Vec3){globalRenderer->cam.up[0],globalRenderer->cam.up[1],globalRenderer->cam.up[2]};
			tmp = WNORM(tmp);
			VEC3TOVEC3F(tmp, globalRenderer->cam.up);


			mat4x4_look_at(globalRenderer->cam.view,
			               globalRenderer->cam.position,
			               globalRenderer->cam.target,
			               globalRenderer->cam.up);

			mat4x4_perspective(globalRenderer->cam.proj,
							   globalRenderer->cam.fov,
							   globalRenderer->cam.aspect,
							   0.1f, 100.0f);
			mat4x4 tmp;
			mat4x4_mul(tmp, globalRenderer->cam.proj, globalRenderer->cam.view);
			mat4x4_mul(globalRenderer->cam.mvp, tmp, globalRenderer->cam.model);

			OEComputeRotationMatrix(globalRenderer->cam.rotation,
				globalRenderer->cam.front, 
				globalRenderer->cam.up);

			break;
	};
}

void OEUpdateViewMat() {
	Camera *cam = &globalRenderer->cam;
	vec3_add(globalRenderer->cam.target, 
				globalRenderer->cam.position, globalRenderer->cam.front);
	if(globalRenderer->camType==PERSPECTIVE) {
		mat4x4_perspective(globalRenderer->cam.proj,
						   globalRenderer->cam.fov,
						   globalRenderer->cam.aspect,
						   0.1f, 100.0f);
	} else if(globalRenderer->camType==ISOMETRIC) {
		float aspect = (float)globalRenderer->window->width / (float)globalRenderer->window->height;
		float oScale = globalRenderer->cam.oScale;
		mat4x4_ortho(globalRenderer->cam.proj, 
				-oScale * aspect, oScale * aspect, 
				-oScale, oScale, 0.1f, 100.0f);

	}
	mat4x4_look_at(cam->view, cam->position, cam->target, cam->up);
	computeCameraRay();
	OEComputeRotationMatrix(globalRenderer->cam.rotation,
			globalRenderer->cam.front, 
			globalRenderer->cam.up);
}

void OEMoveCam(enum face direction, float len) {
	Camera *cam = &globalRenderer->cam;
	vec3 tmp = {0.0f,0.0f,0.0f};
	vec3 forw;
	vec3_dup(forw, cam->front);
	if(globalRenderer->camType==PERSPECTIVE) forw[1]=0.0f;
	switch(direction) {
        case FRONT: {
			if(globalRenderer->camType==ISOMETRIC) {
				cam->position[0] -= len;
				cam->position[1] -= len;
				cam->position[2] -= len;
				vec3_scale(tmp, cam->up, len);
				vec3_add(cam->position, cam->position, tmp);
				break;
			}
            vec3_norm(forw, forw);
            vec3_scale(tmp, forw, len);
            vec3_add(cam->position, cam->position, tmp);
            break;
        }
        case BACKWARD: {
			if(globalRenderer->camType==ISOMETRIC) {
				cam->position[0] += len;
				cam->position[1] += len;
				cam->position[2] += len;
				vec3_scale(tmp, cam->up, -len);
				vec3_add(cam->position, cam->position, tmp);
				break;
			}
            vec3_norm(forw, forw);
            vec3_scale(tmp, forw, -len);
            vec3_add(cam->position, cam->position, tmp);
            break;
        }
		case LEFT: 
			vec3_scale(tmp, cam->right, -len);
        	vec3_add(cam->position, cam->position, tmp);
			break;
		case RIGHT: 
			vec3_scale(tmp, cam->right, len);
        	vec3_add(cam->position, cam->position, tmp);	
			break;
		case UP:
			if(globalRenderer->camType==ISOMETRIC) {
				cam->position[0] += len;
				cam->position[1] += len;
				cam->position[2] += len;
				cam->oScale+=len;break;
			}
			vec3_scale(tmp, cam->up, len);
			vec3_add(cam->position, cam->position, tmp);
			break;
		case DOWN:
			if(globalRenderer->camType==ISOMETRIC) {
				cam->position[0] -= len;
				cam->position[1] -= len;
				cam->position[2] -= len;
				cam->oScale-=len;
				if(cam->oScale<0.1f) cam->oScale=0.1f;
				break;
			}
			vec3_scale(tmp, cam->up, -len);
			vec3_add(cam->position, cam->position, tmp);
			break;
	};

	OEUpdateViewMat();
}

void OECamSet(vec3 pos) {

}

Camera *OEGetCamera() {
	return &globalRenderer->cam;
}

Vec3 OEGetCamPos() {
	return (Vec3){globalRenderer->cam.position[0],
				globalRenderer->cam.position[1],
				globalRenderer->cam.position[2]};
}

SDL_Event OEGetEvent() {
	return globalRenderer->event;
}

int OEIsKeyPressed() {
	return globalRenderer->keyPressed;
}

int OEGetKeySym() {
	return globalRenderer->lastKey;
}

void OEGetMousePos(int *x, int *y) {
	SDL_GetMouseState(x,y);
}

Mouse OEGetMouse() {
	return globalRenderer->mouse;
}

void OEPollEvents(EVENTFUNC event) {
	while(SDL_PollEvent(&globalRenderer->event)!=0) {
		if(globalRenderer->event.type==SDL_QUIT) {
			globalRenderer->window->running = 0;
		}
		if(globalRenderer->event.type==SDL_KEYDOWN) {
			globalRenderer->keyPressed = 1;
			globalRenderer->lastKey = globalRenderer->event.key.keysym.sym;
		} else if(globalRenderer->event.type==SDL_KEYUP) globalRenderer->keyPressed = 0;

	}
	event();
}

int OEGetFPS() {
	return (int)globalRenderer->fps;
}

float OEGetFrameTime() {
	return globalRenderer->frameTime;
}

float OEGetTick() {
	return globalRenderer->tick;
}

SDL_Window *OEGetWindow() {
	return globalRenderer->window->window;
}

void OEAddPostPass(sg_pipeline pipe, UNILOADER loader) {
	if(globalRenderer->postPassSize<MAXPOSTPASS) {
		globalRenderer->postPasses[globalRenderer->postPassSize++] = 
			(PostPass){pipe, loader};
	} else WLOG(WARN, "Too many post passes, skipping.");
}

void OERemovePostPass(UNILOADER loaderloc) {
	int i,j=-1;
	for(i=0;i<globalRenderer->postPassSize;i++) {
		if(globalRenderer->postPasses[i].uniformBind==loaderloc) j=i;
		if(i>=j&&i+1<=globalRenderer->postPassSize) {
			globalRenderer->postPasses[i] = globalRenderer->postPasses[i+1];
		}
	}
	if(j!=-1) {
		globalRenderer->postPasses[globalRenderer->postPassSize] = (PostPass){0};
		globalRenderer->postPassSize--;
	}
}

void OEEnableFXAA() {
	sg_shader fxaa = sg_make_shader(OEFXAA_shader_desc(sg_query_backend()));
	sg_pipeline_desc fxaapd = OEGetQuadPipeline(fxaa, "fxaa");
	sg_pipeline fxaap = sg_make_pipeline(&fxaapd);
	OEAddPostPass(fxaap, (UNILOADER)applyFXAAUniforms);
}

void OEDisableFXAA() {
	OERemovePostPass((UNILOADER)applyFXAAUniforms);
}

void OEUpdateBloomParams(float threshold, float strength) {
	globalRenderer->bloomParams.thresh = threshold;
	globalRenderer->bloomParams.strength = strength;
}

void OEEnableBloom(float threshold, float strength) {
	sg_shader bloom = sg_make_shader(OEBQuad_shader_desc(sg_query_backend()));
	sg_pipeline_desc bloompd = OEGetQuadPipeline(bloom, "bloom");
	sg_pipeline bloomp = sg_make_pipeline(&bloompd);
	globalRenderer->bloomParams.resolution[0] = globalRenderer->window->width;
	globalRenderer->bloomParams.resolution[1] = globalRenderer->window->height;
	globalRenderer->bloomParams.thresh = threshold;
	globalRenderer->bloomParams.strength = strength;
	OEAddPostPass(bloomp, (UNILOADER)applyBloomUniforms);
}

void OEDisableBloom() {
	OERemovePostPass((UNILOADER)applyBloomUniforms);
}

/*void OEEnableSSAO(float strength) {
	sg_shader ssao = sg_make_shader(OESSAO_shader_desc(sg_query_backend()));
	sg_pipeline_desc ssaopd = OEGetQuadPipeline(ssao, "ssao");
	sg_pipeline ssaop = sg_make_pipeline(&ssaopd);
	globalRenderer->ssao.params.resolution[0] = globalRenderer->window->width;
	globalRenderer->ssao.params.resolution[1] = globalRenderer->window->height;
	globalRenderer->ssao.params.strength = strength;
	OEAddPostPass(ssaop, (UNILOADER)applySSAOUniforms);
}

void OEDisableSSAO() {
	OERemovePostPass((UNILOADER)applySSAOUniforms);
}*/

void OERenderFrame(RENDFUNC drawCall) {
	globalRenderer->frame_start = SDL_GetPerformanceCounter();
	SDL_GetWindowSize(globalRenderer->window->window,
			&globalRenderer->window->width,
			&globalRenderer->window->height);
	OEGetMousePos(&globalRenderer->mouse.posx, &globalRenderer->mouse.posy);
	//globalRenderer->frameTime = sapp_frame_duration() * 1000.0;
	sg_pass_action pass_action = (sg_pass_action) {
       	.colors[0] = {
           	.load_action = SG_LOADACTION_CLEAR,
       		.clear_value = {0.0f,0.0f,0.0f,1.0f}
        },

    };
	sg_pass_action off_pass_action = (sg_pass_action) {
       	.colors = {
           	[0] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[1] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {1.0f,1.0f,1.0f,1.0f}},
			[2] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {1.0f,1.0f,1.0f,1.0f}},
			[3] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {1.0f,1.0f,1.0f,1.0f}}
        },
    };
	sg_pass_action post_pass_action = (sg_pass_action) {
		.colors[0].load_action = SG_LOADACTION_DONTCARE,
	};
	/*Offscreen pass to the texture*/
    sg_begin_pass(&(sg_pass){ .action = off_pass_action,
			.attachments = globalRenderer->renderTargetAtt});
	
	drawCall();	
	sg_end_pass();

	sg_image src = globalRenderer->renderTarget;
	sg_image final = src;

	/*Post-passes*/
	int i;
	for(i=0;i<globalRenderer->postPassSize;i++) {
		sg_image dst = (src.id==globalRenderer->renderTarget.id||
				src.id==globalRenderer->postTargetPong.id)
				?globalRenderer->postTarget:globalRenderer->postTargetPong;
		sg_attachments dstAtt = (src.id==globalRenderer->renderTarget.id||
				src.id==globalRenderer->postTargetPong.id)
				?globalRenderer->postTargetAtt:globalRenderer->postTargetAttPong;

		sg_begin_pass(&(sg_pass){ .action = post_pass_action,
				.attachments = dstAtt});

		sg_apply_pipeline(globalRenderer->postPasses[i].pipe);
		sg_apply_bindings(&(sg_bindings){
			.vertex_buffers[0] = globalRenderer->renderTargetBuff,
			.images[IMG_OEquad_texture] = src,
			.images[1] = globalRenderer->depthBuffer,
			.images[2] = globalRenderer->normalBuffer,
			.images[3] = globalRenderer->positionBuffer,
			.samplers[SMP_OEquad_smp] = globalRenderer->ssao.sampler,
		});
		if(globalRenderer->postPasses[i].uniformBind!=NULL) 
			globalRenderer->postPasses[i].uniformBind();
		sg_draw(0,6,1);

		sg_end_pass();
		src = dst;
		final = src;
	}

	/*On-Screen main pass*/
	sg_begin_pass(&(sg_pass){ .action = pass_action,
			.swapchain = OEGetSwapChain()});

	sg_apply_pipeline(globalRenderer->renderTargetPipe);
	sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = globalRenderer->renderTargetBuff,
		.images[0] = final,
		.samplers[0] = globalRenderer->ssao.sampler
	});
	sg_draw(0,6,1);

	if(globalRenderer->debug) {
		sdtx_canvas(globalRenderer->window->width * 0.5f, 
				globalRenderer->window->height * 0.5f);
    	sdtx_origin(1.0f, 1.0f);
		sdtx_font(1);
		sdtx_color3b(0xf4, 0x43, 0x36);
		Camera *cam = &globalRenderer->cam;
		sdtx_printf("FPS: %d\nFrameTime: %f\nCamPos: %f, %f, %f", 
				(int)globalRenderer->fps, globalRenderer->frameTime,
				cam->position[0], cam->position[1], cam->position[2]);
		sdtx_draw();
	}

	
	sg_end_pass();

	sg_commit();

	/*glClear(GL_COLOR_BUFFER_BIT);*/
	SDL_GL_SwapWindow(globalRenderer->window->window);

	OEUpdateViewMat();

	globalRenderer->frame_end = SDL_GetPerformanceCounter();
	globalRenderer->fps =
		1.0f/((globalRenderer->frame_end - globalRenderer->frame_start) / 
		(float)SDL_GetPerformanceFrequency());
	globalRenderer->frameTime = 1.0f/globalRenderer->fps;
	globalRenderer->tick+=globalRenderer->frameTime;
}

/*These functions are pretty much just for the openxr stuff but,
 * if for some reason you want to make a custom render loop you'll need these I guess.*/
void OERendererTimerStart() {
	globalRenderer->frame_start = SDL_GetPerformanceCounter();
	SDL_GetWindowSize(globalRenderer->window->window,
			&globalRenderer->window->width,
			&globalRenderer->window->height);
	OEGetMousePos(&globalRenderer->mouse.posx, &globalRenderer->mouse.posy);
}

void OERendererTimerEnd() {
	globalRenderer->frame_end = SDL_GetPerformanceCounter();
	globalRenderer->fps =
		1.0f/((globalRenderer->frame_end - globalRenderer->frame_start) / 
		(float)SDL_GetPerformanceFrequency());
	globalRenderer->frameTime = 1.0f/globalRenderer->fps;
	globalRenderer->tick+=globalRenderer->frameTime;

}

void OECleanup(void) {
	sg_shutdown();
	SDL_DestroyWindow(globalRenderer->window->window);
	SDL_Quit();
}
