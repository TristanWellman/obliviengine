/*Copyright (c) 2025 Tristan Wellman*/
#define SOKOL_IMPL
#include <OE/OE.h>
#include <OE/cube.h>
#include <stb/stb_image.h>

#ifndef igGetIO
#define igGetIO igGetIO_Nil
#endif

/*
 * OE lib functions
 * */

_OE_HOT Object *OEGetObjectFromName(char *name) {
	if(name==NULL) return NULL;
	Object key = (Object){.name = calloc(strlen(name)+1, sizeof(char))};
	strcpy(key.name, name);
	Object *res = (Object *)bsearch(&key, globalRenderer->objects,
			globalRenderer->objSize, sizeof(Object), OEObjectCmpName);
	free(key.name);
	key.name = NULL;
	return res;
}

_OE_HOT OEInstanceBatch *OEGetInstanceBatchFromName(char *name) {
	if(name==NULL) return NULL;
	OEInstanceBatch key;
	key.obj = calloc(1, sizeof(Object));
	key.obj->name = calloc(strlen(name)+1, sizeof(char));
	strcpy(key.obj->name, name);
	OEInstanceBatch *res = (OEInstanceBatch *)bsearch(&key, globalRenderer->iBatches,
			globalRenderer->iBatchSize, sizeof(OEInstanceBatch), OEIBatchCmpName);
	free(key.obj->name);
	key.obj->name = NULL;
	free(key.obj);
	key.obj = NULL;
	return res;
}

void OEAttachScript(char *ID, char *scriptPath) {
	FILE *test = fopen(scriptPath, "r");
	if(test==NULL) {
		int ssize = strlen(scriptPath)+1024;
		char *buf = calloc(ssize, sizeof(char));
		snprintf(buf, sizeof(char)*ssize, "Failed to open Lua script: %s", scriptPath);
		WLOG(WARN, buf);
		free(buf); buf = NULL;
		fclose(test);
		return;
	}
	fclose(test);
	Object *obj = OEGetObjectFromName(ID);
	obj->script = (OEScript){scriptPath};
}

void preFrameScriptExecute() {
	int i;
	lua_State *state = globalRenderer->luaData.lState;
	for(i=0;i<globalRenderer->objSize;i++) {
		if(globalRenderer->objects[i].script.filePath!=NULL) {
			if(luaL_dofile(state, globalRenderer->objects[i].script.filePath))
				lua_pop(state, lua_gettop(state));
		}
	}
}

_OE_HOT void OECreateInstanceBatch(Object *obj) {
	if(obj==NULL) return;
	if(obj->name==NULL) return;
	if(OEGetInstanceBatchFromName(obj->name)) {
		char buf[strlen(obj->name)+128];
		sprintf(buf, "Duplicate Instance Batch: %s. Not adding.", obj->name);
		WLOG(WARN, buf);
		return;
	}

	if(globalRenderer->iBatchSize>=globalRenderer->iBatchCap) {
		globalRenderer->iBatchCap += OBJSTEP;
		globalRenderer->iBatches = 
			(OEInstanceBatch *)realloc(globalRenderer->iBatches, 
					sizeof(OEInstanceBatch)*globalRenderer->iBatchCap);
	}
	
	globalRenderer->iBatches[globalRenderer->iBatchSize].obj = obj;
	globalRenderer->iBatches[globalRenderer->iBatchSize].size = 0;
	globalRenderer->iBatches[globalRenderer->iBatchSize].instModelsr0 = calloc(INSTMAX, sizeof(vec4));
	globalRenderer->iBatches[globalRenderer->iBatchSize].instModelsr1 = calloc(INSTMAX, sizeof(vec4));
	globalRenderer->iBatches[globalRenderer->iBatchSize].instModelsr2 = calloc(INSTMAX, sizeof(vec4));
	globalRenderer->iBatches[globalRenderer->iBatchSize].instModelsr3 = calloc(INSTMAX, sizeof(vec4));
	globalRenderer->iBatches[globalRenderer->iBatchSize].mbuf0 = sg_make_buffer(&(sg_buffer_desc){
				.size = INSTMAX*sizeof(vec4),
				.usage.stream_update = true,
				.label = obj->name
			});
	globalRenderer->iBatches[globalRenderer->iBatchSize].mbuf1 = sg_make_buffer(&(sg_buffer_desc){
				.size = INSTMAX*sizeof(vec4),
				.usage.stream_update = true,
				.label = obj->name
			});
	globalRenderer->iBatches[globalRenderer->iBatchSize].mbuf2 = sg_make_buffer(&(sg_buffer_desc){
				.size = INSTMAX*sizeof(vec4),
				.usage.stream_update = true,
				.label = obj->name
			});
	globalRenderer->iBatches[globalRenderer->iBatchSize].mbuf3 = sg_make_buffer(&(sg_buffer_desc){
				.size = INSTMAX*sizeof(vec4),
				.usage.stream_update = true,
				.label = obj->name
			});
	globalRenderer->iBatchSize++;
	OEQSortIBatch();
}

_OE_HOT void OEPushInstanceBatchData(OEInstanceBatch *batch, vec3 pos) {
	if(batch==NULL) return;
	if(batch->size>=INSTMAX) {
		WLOG(WARN, "Instance Batch too large, not adding.");
		return;
	}
	mat4x4 objModel;
	mat4x4_identity(objModel);
	mat4x4_translate(objModel, pos[0], pos[1], pos[2]);
	vec4_dup(batch->instModelsr0[batch->size], objModel[0]);
	vec4_dup(batch->instModelsr1[batch->size], objModel[1]);
	vec4_dup(batch->instModelsr2[batch->size], objModel[2]);
	vec4_dup(batch->instModelsr3[batch->size], objModel[3]);
	batch->size++;
}

void OECreateObject(Object obj) {
	if(obj.name==NULL) return;
	int i;
	/*Check for dups*/
	if(OEGetObjectFromName(obj.name)) {
		char buf[strlen(obj.name)+128];
		sprintf(buf, "Duplicate object names: %s. Not adding.", obj.name);
		WLOG(WARN, buf);
		return;
	}

	if(globalRenderer->objSize>=globalRenderer->objCap) {
		globalRenderer->objCap+=OBJSTEP;
		globalRenderer->objects =
			(Object *)realloc(globalRenderer->objects, 
					sizeof(Object)*globalRenderer->objCap);
	}
	for(i=0;i<globalRenderer->objSize&&globalRenderer->objects[i].name!=NULL;i++);
	size_t pos = globalRenderer->objSize;
	globalRenderer->objects[pos].vrtPtr = obj.vrtPtr;
	globalRenderer->objects[pos].vrtSize = obj.vrtSize;
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
	OEQSortObjects();

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
	free(obj.name); obj.name = NULL;
}

void OECreateObjectFromMesh(OEMesh *mesh, vec3 pos
		/*sg_shader defShader, sg_pipeline_desc pipe*/) {
	if(mesh==NULL) return;
	Object obj = {0};
	obj.name = calloc(strlen(mesh->label)+1, sizeof(char));
	strcpy(obj.name, mesh->label);


	int numFaces = mesh->indices.size;
	int totalVerts = numFaces*ISIZE;
	int totalTris = numFaces*2;
	int fpv = VSIZE+4+NORMSIZE+TEXSIZE;
	int vertSize = totalVerts*fpv;
	int indSize = totalTris*3;
	obj.numIndices = indSize;
	float *finalVerts = calloc(vertSize, sizeof(float));
	uint16_t *finalInds = calloc(indSize, sizeof(uint16_t));

	int i,j,l=0;
	for(i=0;i<numFaces;i++) {
		for(j=0;j<ISIZE;j++,l++) {
			int b = l*fpv;
			/*Verts*/
			int v = mesh->indices.data[i][j];
			int vI = v<0?mesh->verts.size+v:v-1;
			if(vI<0||vI>=mesh->verts.size) vI = 0;
			finalVerts[b] = mesh->verts.data[vI][0];
			finalVerts[b+1] = mesh->verts.data[vI][1];
			finalVerts[b+2] = mesh->verts.data[vI][2];
			/*Color*/
			finalVerts[b+3] = 1.0f;
			finalVerts[b+4] = 1.0f;
			finalVerts[b+5] = 1.0f;
			finalVerts[b+6] = 1.0f;
			/*Norms*/
			int vn = mesh->normInds.data[i][j];
			float nx = 0.0f, ny = 0.0f, nz = 0.0f;
			if(vn!=0) {
				int vnI = vn<0?mesh->vertNorms.size+vn:vn-1;
				if(vnI>=0&&vnI<mesh->vertNorms.size) {
					nx = mesh->vertNorms.data[vnI][0];
					ny = mesh->vertNorms.data[vnI][1];
					nz = mesh->vertNorms.data[vnI][2];
				}
			}
			finalVerts[b+7] = nx;
			finalVerts[b+8] = ny;
			finalVerts[b+9] = nz;
			/*UV/texcoords*/
			int vt = mesh->texInds.data[i][j];
			float tu = 0.0f, tv = 0.0f;
			if(vt!=0) {
				int vtI = vt<0?mesh->vertTex.size+vt:vt-1;
				if(vtI>=0&&vtI<mesh->vertTex.size) {
					tu = mesh->vertTex.data[vtI][0];
					tv = mesh->vertTex.data[vtI][1];
				}
			}
			finalVerts[b+10] = tu;
			finalVerts[b+11] = tv;
		}
	}
	int offs = 0;
	for(i=0,j=0;i<numFaces;i++,offs+=ISIZE) {
		finalInds[j++] = (uint16_t)(offs);
		finalInds[j++] = (uint16_t)(offs+2);
		finalInds[j++] = (uint16_t)(offs+1);
		finalInds[j++] = (uint16_t)(offs);
		finalInds[j++] = (uint16_t)(offs+3);
		finalInds[j++] = (uint16_t)(offs+2);
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
	free(obj.name); obj.name = NULL;
	free(finalVerts); finalVerts = NULL;
	free(finalInds); finalInds = NULL;
}

/*This should be used over the OECreateObjectFromMesh since my .obj parser is "ok"*/
void OECreateMeshFromAssimp(char *name, char *path, vec3 pos, int flag) {
	Object obj = {0};
	obj.name = strdup(name); 

	const struct aiScene *scene = aiImportFile(path,
			aiProcess_GenSmoothNormals|aiProcess_Triangulate|
			aiProcess_JoinIdenticalVertices|aiProcess_PreTransformVertices);
	if(!scene||scene->mNumMeshes==0) {
		char *buf = calloc(128+strlen(path), sizeof(char));
		snprintf(buf,sizeof(buf),"Failed to load FBX file: %s", path);
		WLOG(ERROR,buf)
		free(buf); buf = NULL;
		return;
	}
	struct aiMesh *mesh = scene->mMeshes[0];
	int faceCount = mesh->mNumFaces;
	int vertCount = faceCount*VSIZE;
	int indCount = faceCount*VSIZE;
	int totalFSize = 12*vertCount; /*all the floats*/
	obj.numIndices = indCount;

	float *finalVerts = calloc(totalFSize, sizeof(float));
	uint16_t *finalInds = calloc(indCount, sizeof(uint16_t));

	float scale = 0.01f;

	int i,j,k,l=0;
	for(i=0;i<faceCount;i++) {
		struct aiFace *face = &mesh->mFaces[i];
		for(j=0;j<VSIZE;j++,l++) {
			/*position*/
			k = face->mIndices[j];
			int b = l*12;
			finalVerts[b+0] = mesh->mVertices[k].x*scale;
			finalVerts[b+1] = mesh->mVertices[k].y*scale;
			finalVerts[b+2] = mesh->mVertices[k].z*scale;
			/*Color*/
			finalVerts[b+3] = 1.0f;
			finalVerts[b+4] = 1.0f;
			finalVerts[b+5] = 1.0f;
			finalVerts[b+6] = 0.0f;
			/*Normals*/
			finalVerts[b+7] = mesh->mNormals[k].x;
			finalVerts[b+8] = mesh->mNormals[k].y;
			finalVerts[b+9] = mesh->mNormals[k].z;
			/*UV/texcoords*/
			if(mesh->mTextureCoords[0]) {
				finalVerts[b+10] = mesh->mTextureCoords[0][k].x;
				finalVerts[b+11] = 1.0-mesh->mTextureCoords[0][k].y;
			} else {
				finalVerts[b+10] = 0.0f;
				finalVerts[b+11] = 0.0f;
			}
		}
	}

	for(i=0;i<faceCount;i++) {
		finalInds[(i*VSIZE)+0] = (uint16_t)((i*VSIZE)+0);
		finalInds[(i*VSIZE)+1] = (uint16_t)((i*VSIZE)+2);
		finalInds[(i*VSIZE)+2] = (uint16_t)((i*VSIZE)+1);
	}

	if(flag==OE_KEEPVERTS) {
		obj.vrtPtr = calloc(totalFSize+1, sizeof(float));
		memcpy(obj.vrtPtr, finalVerts, totalFSize*sizeof(float));
		obj.vrtSize = totalFSize;
	}
	obj.vbuf = sg_make_buffer(&(sg_buffer_desc) {
				.data = PTRRANGE(finalVerts, totalFSize),
				.label = "objv"
			});
	obj.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.usage.index_buffer = true,
				.data = PTRRANGE(finalInds, indCount),
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
	free(finalVerts); finalVerts = NULL;
	free(finalInds); finalInds = NULL;
	aiReleaseImport(scene);
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

_OE_HOT Vec3 OEGetObjectPosition(char *ID) {
	Object *obj = OEGetObjectFromName(ID);
	Vec3 ret = {0, 0, 0};
	if(obj!=NULL) {
		ret.x = obj->pos[0];
		ret.y = obj->pos[1];
		ret.z = obj->pos[2];
	} else WLOG(WARN, "Getting invalid object position!");
	return ret;
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
	vs_params_t vs_params = {.tick = OEGetTick()};
	fs_params_t fs_params = {.numLights = getNumLights()};
	light_params_t light_params = getLightUniform();

    mat4x4 mvp, mv;
    mat4x4_mul(mv, globalRenderer->cam.view, obj->model);
    mat4x4_mul(mvp, globalRenderer->cam.proj, mv);

	Camera *cam = OEGetCamera();
    //memcpy(vs_params.mvp, mvp, sizeof(mvp));
	memcpy(vs_params.model, obj->model, sizeof(obj->model));
	memcpy(vs_params.view, cam->view, sizeof(cam->view));
	memcpy(vs_params.proj, cam->proj, sizeof(cam->proj));
	memcpy(fs_params.camPos, cam->position, sizeof(cam->position));

	if(globalRenderer->graphicsSetting>OE_LOW_GRAPHICS) {
		sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
		sg_apply_uniforms(UB_fs_params, &SG_RANGE(fs_params));
		sg_apply_uniforms(UB_light_params, &SG_RANGE(light_params));
	} else {
		OELOW_vs_params_t vslow;
		OELOW_fs_params_t fslow;
		OELOW_light_params_t lightlow;
		memcpy(&vslow, &vs_params, sizeof(vslow));
		memcpy(&fslow, &fs_params, sizeof(fslow));
		memcpy(&lightlow, &light_params, sizeof(lightlow));
		sg_apply_uniforms(UB_OELOW_vs_params, &SG_RANGE(vslow));
		sg_apply_uniforms(UB_OELOW_fs_params, &SG_RANGE(fslow));
		sg_apply_uniforms(UB_OELOW_light_params, &SG_RANGE(lightlow));
	}
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

void *applySSAOUniforms() {
	memcpy(globalRenderer->ssaoParams.proj, 
			globalRenderer->cam.proj, sizeof(globalRenderer->ssgiParams.proj));
	sg_apply_uniforms(UB_OESSAO_params, &SG_RANGE(globalRenderer->ssaoParams));
	return NULL;
}

void *applySSGIUniforms() {
	memcpy(globalRenderer->ssgiParams.proj, 
			globalRenderer->cam.proj, sizeof(globalRenderer->ssgiParams.proj));
	sg_apply_uniforms(UB_OESSGI_params, &SG_RANGE(globalRenderer->ssgiParams));
	return NULL;
}

void *applyDnoiseUniforms() {
	sg_apply_uniforms(UB_OEDNOISE_params, &SG_RANGE(globalRenderer->deNoiseParams));
	return NULL;
}

void runObjLuaScript(Object *obj) {
	lua_State *state = globalRenderer->luaData.lState;
	if(obj->script.filePath!=NULL) {
		if(luaL_dofile(state, obj->script.filePath))
			lua_pop(state, lua_gettop(state));
	}
}

_OE_HOT void OEDrawInstanceBatch(OEInstanceBatch *batch) {
	if(batch==NULL) {
		WLOG(ERROR, "NULL instance batch passed to drawInstanceBatch");
		return;
	}
	if(batch->size<=0) return;

	sg_update_buffer(batch->mbuf0, &(sg_range){
				.ptr = batch->instModelsr0,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf1, &(sg_range){
				.ptr = batch->instModelsr1,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf2, &(sg_range){
				.ptr = batch->instModelsr2,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf3, &(sg_range){
				.ptr = batch->instModelsr3,
				.size = (size_t)batch->size*sizeof(vec4)
	});

	sg_apply_pipeline(globalRenderer->iBatchPipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = batch->obj->vbuf,
		.vertex_buffers[1] = batch->mbuf0,
		.vertex_buffers[2] = batch->mbuf1,
		.vertex_buffers[3] = batch->mbuf2,
		.vertex_buffers[4] = batch->mbuf3,
        .index_buffer = batch->obj->ibuf,
		.views[OE_TEXPOS] = OEGetDefaultTexture(),
        .samplers[OE_TEXPOS] = globalRenderer->sampler
    });

	OEApplyCurrentUniforms(batch->obj);
    sg_draw(0, batch->obj->numIndices, (int)batch->size);
}

_OE_HOT void OEDrawInstanceBatchTex(OEInstanceBatch *batch,
		int assign, sg_view texture) {
	if(batch==NULL) {
		WLOG(ERROR, "NULL instance batch passed to drawInstanceBatch");
		return;
	}
	if(batch->size<=0) return;

	sg_update_buffer(batch->mbuf0, &(sg_range){
				.ptr = batch->instModelsr0,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf1, &(sg_range){
				.ptr = batch->instModelsr1,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf2, &(sg_range){
				.ptr = batch->instModelsr2,
				.size = (size_t)batch->size*sizeof(vec4)
	});
	sg_update_buffer(batch->mbuf3, &(sg_range){
				.ptr = batch->instModelsr3,
				.size = (size_t)batch->size*sizeof(vec4)
	});

	sg_apply_pipeline(globalRenderer->iBatchPipe);
    sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = batch->obj->vbuf,
		.vertex_buffers[1] = batch->mbuf0,
		.vertex_buffers[2] = batch->mbuf1,
		.vertex_buffers[3] = batch->mbuf2,
		.vertex_buffers[4] = batch->mbuf3,
        .index_buffer = batch->obj->ibuf,
		.views[3] = texture,
        .samplers[3] = globalRenderer->sampler
    });

	OEApplyCurrentUniforms(batch->obj);
    sg_draw(0, batch->obj->numIndices, (int)batch->size);
}


_OE_HOT void OEDrawObject(Object *obj) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}
	
	runObjLuaScript(obj);

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.views[OE_TEXPOS] = OEGetDefaultTexture(),
        .samplers[OE_TEXPOS] = globalRenderer->sampler

    });

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectBind(Object *obj, sg_bindings binding) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

	runObjLuaScript(obj);
    
	sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&binding);

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}


_OE_HOT void OEDrawObjectTex(Object *obj, int assign, sg_view texture) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

	runObjLuaScript(obj);

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.views[3] = texture,
        .samplers[3] = globalRenderer->sampler
    });

	OEApplyCurrentUniforms(obj);
    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectEx(Object *obj, UNILOADER apply_uniforms) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

	runObjLuaScript(obj);

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.views[OE_TEXPOS] = OEGetDefaultTexture(),
        .samplers[OE_TEXPOS] = globalRenderer->sampler
    });

	apply_uniforms();

    sg_draw(0, obj->numIndices, 1);
}

void OEDrawObjectTexEx(Object *obj, int assign,
		sg_view texture, UNILOADER apply_uniforms) {
	if(obj==NULL) {
		WLOG(ERROR, "NULL object passed to drawObject");
		return;
	}

	runObjLuaScript(obj);

    sg_apply_pipeline(obj->pipe);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = obj->vbuf,
        .index_buffer = obj->ibuf,
		.views[3] = texture,
        .samplers[3] = globalRenderer->sampler
    });

	apply_uniforms();

    sg_draw(0, obj->numIndices, 1);
}

_OE_PURE int OERendererIsRunning() {
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

_OE_PURE _OE_COLD sg_environment OEGetEnv() {
#ifndef OE_VULKAN
	return (sg_environment) {
		.defaults = {
			.color_format = SG_PIXELFORMAT_RGBA32F,
			.depth_format = SG_PIXELFORMAT_DEPTH,
			.sample_count = 1
		},
	};
#else
	return (sg_environment) {
		.defaults = {
			.color_format = SG_PIXELFORMAT_RGBA32F,
			.depth_format = SG_PIXELFORMAT_DEPTH,
			.sample_count = 1,
		},
		.vulkan = {
			.instance = (const void *)globalRenderer->window->VK->instance,
			.physical_device = (const void *)globalRenderer->window->VK->physDevice,
			.device = (const void *)globalRenderer->window->VK->device,
			.queue = (const void *)globalRenderer->window->VK->queue,
			.queue_family_index = globalRenderer->window->VK->queueFamIndex	
		}
	};
#endif
}

_OE_COLD sg_swapchain OEGetSwapChain() {
	int w = globalRenderer->window->width;
	int h = globalRenderer->window->height;
#ifndef OE_VULKAN
	return (sg_swapchain) {
		.sample_count = 1,
		.color_format = SG_PIXELFORMAT_RGBA32F,
		.depth_format = SG_PIXELFORMAT_DEPTH,
		.width = w,
		.height = h,
		.gl.framebuffer = 0,
	};
#else
	OEVKData *vk = globalRenderer->window->VK;
	return (sg_swapchain) {
		.sample_count = 1,
		.color_format = SG_PIXELFORMAT_RGBA32F,
		.depth_format = SG_PIXELFORMAT_DEPTH,
		.width = w,
		.height = h,
		.vulkan = {
			.render_image = (const void *)vk->images.images[vk->images.imageIndex],
			.render_view = (const void *)vk->images.views[vk->images.imageIndex],
			.resolve_image = NULL,
			.resolve_view = NULL,
			.depth_stencil_image = (const void *)vk->images.depthImage,
			.depth_stencil_view = (const void *)vk->images.depthImageView,
			.render_finished_semaphore = (const void *)vk->images.renderSema[vk->images.imageIndex],
			.present_complete_semaphore = (const void *)vk->images.presentSema[vk->images.semaSlotIndex],
		}
	};
#endif
}

sg_pipeline_desc OEGetDefaultPipe(sg_shader shader, char *label) {
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
			.color_count = 6,
			.colors = {
				[0] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Color Buffer*/
				[1] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Depth Buffer*/
				[2] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Normal Buffer*/
				[3] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Position Buffer*/
				[4] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Noise Buffer*/
				[5] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}  /*Previous Frame Buffer*/
			},
			.colors[0].blend = (sg_blend_state){
				.enabled = true,
				.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
				.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				.op_rgb = SG_BLENDOP_ADD,
				.src_factor_alpha = SG_BLENDFACTOR_ONE,
				.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				.op_alpha = SG_BLENDOP_ADD,
			},
        	.depth = {
				.compare = SG_COMPAREFUNC_LESS_EQUAL,
				.pixel_format = SG_PIXELFORMAT_DEPTH,
				.write_enabled = true
        	},
			.label = label,
#ifdef OE_VULKAN
			.face_winding = SG_FACEWINDING_CCW
#endif
		};
}

sg_pipeline_desc OEGetInstancingPipe(sg_shader shader, char *label) {
	return (sg_pipeline_desc){
			.shader = shader,
			.layout = {
				.buffers = {
					[0] = {.step_func=SG_VERTEXSTEP_PER_VERTEX},
					[1] = {.step_func=SG_VERTEXSTEP_PER_INSTANCE},
					[2] = {.step_func=SG_VERTEXSTEP_PER_INSTANCE},
					[3] = {.step_func=SG_VERTEXSTEP_PER_INSTANCE},
					[4] = {.step_func=SG_VERTEXSTEP_PER_INSTANCE}, 
					[5] = {.step_func=SG_VERTEXSTEP_PER_INSTANCE}
				},
            	.attrs = {
                	[ATTR_simpleInst_position] = 
						{.format=SG_VERTEXFORMAT_FLOAT3,.buffer_index=0},
					[ATTR_simple_color0] = 
						{.format=SG_VERTEXFORMAT_FLOAT4,.buffer_index=0},
					[ATTR_simple_normal0] = 
						{.format=SG_VERTEXFORMAT_FLOAT3,.buffer_index=0},
					[ATTR_simple_texcoord0] = 
						{.format=SG_VERTEXFORMAT_FLOAT2,.buffer_index=0},
					[ATTR_simpleInst_instModelr0] =
						{.format=SG_VERTEXFORMAT_FLOAT4,.buffer_index=1},
            		[ATTR_simpleInst_instModelr1] =
						{.format=SG_VERTEXFORMAT_FLOAT4,.buffer_index=2},
					[ATTR_simpleInst_instModelr2] =
						{.format=SG_VERTEXFORMAT_FLOAT4,.buffer_index=3},
					[ATTR_simpleInst_instModelr3] =
						{.format=SG_VERTEXFORMAT_FLOAT4,.buffer_index=4},
				}
        	},
			.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
			.index_type = SG_INDEXTYPE_UINT16,
        	.cull_mode = SG_CULLMODE_BACK,
			.sample_count = 1,
			.color_count = 6,
			.colors = {
				[0] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Color Buffer*/
				[1] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Depth Buffer*/
				[2] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Normal Buffer*/
				[3] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Position Buffer*/
				[4] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}, /*Noise Buffer*/
				[5] = {.pixel_format = SG_PIXELFORMAT_RGBA32F}  /*Previous Frame Buffer*/
			},
			.colors[0].blend = (sg_blend_state){
				.enabled = true,
				.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
				.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				.op_rgb = SG_BLENDOP_ADD,
				.src_factor_alpha = SG_BLENDFACTOR_ONE,
				.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
				.op_alpha = SG_BLENDOP_ADD,
			},
        	.depth = {
				.compare = SG_COMPAREFUNC_LESS_EQUAL,
				.pixel_format = SG_PIXELFORMAT_DEPTH,
				.write_enabled = true
        	},
			.label = label,
#ifdef OE_VULKAN
			.face_winding = SG_FACEWINDING_CCW
#endif
		};
}

sg_pipeline_desc OEGetQuadPipeline(sg_shader shader, char *label) {
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
		.color_count = 1,
		.colors[0].pixel_format = SG_PIXELFORMAT_RGBA32F,
		.colors[0].blend = (sg_blend_state){
			.enabled = true,
			.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
			.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.op_rgb = SG_BLENDOP_ADD,
			.src_factor_alpha = SG_BLENDFACTOR_ONE,
			.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.op_alpha = SG_BLENDOP_ADD,
		},
        .depth = {
			.compare = SG_COMPAREFUNC_ALWAYS,
			.write_enabled = false,
        },
		.label = label,
#ifdef OE_VULKAN
			.face_winding = SG_FACEWINDING_CCW
#endif
	};
}

sg_shader OEGetDefCubeShader() {
	return globalRenderer->defCubeShader;
}

sg_shader OEGetDefInstShader() {
	return globalRenderer->originalIBatchShader;
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

void OESetDefaultInstancingShader(sg_shader shader) {
	sg_destroy_pipeline(globalRenderer->iBatchPipe);
	sg_pipeline_desc pipe = OEGetInstancingPipe(shader, "InstancePipe");
	globalRenderer->iBatchPipe = sg_make_pipeline(&pipe);
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

_OE_PURE sg_view OEGetDefaultTexture() {
	return globalRenderer->defTexture;
}

_OE_PURE sg_sampler OEGetSampler() {
	return globalRenderer->sampler;
}

_OE_PURE sg_pipeline OEGetRTP() {
	return globalRenderer->renderTargetPipe;
}

void initBaseObjects() {

	/*Create a test light*/
	/*Color c = (Color){255.0f, 135.0f, 102.0f, 255.0f};
	OEAddLight("Light1", (vec3){-5.0f, 5.0f, -5.0f},
			RGBA255TORGBA1(c));*/

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
}

_OE_COLD void OEEnableDebugInfo() {
	globalRenderer->debug = 1;
}

_OE_COLD void OEDisableDebugInfo() {
	globalRenderer->debug = 0;
}

_OE_COLD void OEDisableSdtx() {
	globalRenderer->disablesdtx = 1;
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

int OEDumpSupportedPixelFormats() {
	int ret = 1;
	sg_pixelformat_info pfinfo = sg_query_pixelformat(SG_PIXELFORMAT_RGBA32F);
	if(pfinfo.render) {WLOG(PF_INFO, "SG_PIXELFORMAT_RGBA32F: renderable");}
	else {WLOG(PF_INFO, "SG_PIXELFORMAT_RGBA32F: un-renderable");ret=0;}
	pfinfo = sg_query_pixelformat(SG_PIXELFORMAT_RGBA32F);
	if(pfinfo.render) {WLOG(PF_INFO, "SG_PIXELFORMAT_RGBA32F: renderable");}
	else {WLOG(PF_INFO, "SG_PIXELFORMAT_RGBA32F: un-renderable");ret=0;}
	pfinfo = sg_query_pixelformat(SG_PIXELFORMAT_DEPTH);
	if(pfinfo.render) {WLOG(PF_INFO, "SG_PIXELFORMAT_DEPTH: renderable");}
	else {WLOG(PF_INFO, "SG_PIXELFORMAT_DEPTH: un-renderable");ret=0;}
	return ret;
}

_OE_COLD void OEGetGLVersion(int *_OE_RESTRICT maj, int *_OE_RESTRICT min) {
	glGetIntegerv(GL_MAJOR_VERSION, maj);
	glGetIntegerv(GL_MINOR_VERSION, min);
}

_OE_COLD void OEGLFallbackInit() {
	SDL_GL_DeleteContext(globalRenderer->window->gl_context);
	SDL_DestroyWindow(globalRenderer->window->window);
	SDL_GL_UnloadLibrary();
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Quit();

	WASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)>=0,
			"ERROR:: Failed to init SDL!");


	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	

	WASSERT(SDL_GL_LoadLibrary(NULL)==0,
			"Failed to load GL library!");

	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");

	globalRenderer->window->window = SDL_CreateWindow(
			globalRenderer->window->title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			globalRenderer->window->width, globalRenderer->window->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	globalRenderer->window->gl_context = 
		SDL_GL_CreateContext(globalRenderer->window->window);
	WASSERT(globalRenderer->window->gl_context!=NULL,
			"ERROR:: Failed to init gl context");
	WASSERT(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress),
		"ERROR:: Failed to initialize GLAD!");
	globalRenderer->legacy = 1;
	globalRenderer->graphicsSetting = OE_LOW_GRAPHICS;
	WLOG(SDL_INFO, SDL_GetError());
}

void OESetRenderResolution(int w, int h) {
	if(w==globalRenderer->window->renderWidth&&
			h==globalRenderer->window->renderHeight) return;
	sg_destroy_image(globalRenderer->renderTarget);
	sg_destroy_image(globalRenderer->postTarget);
	sg_destroy_image(globalRenderer->postTargetPong);
	sg_destroy_image(globalRenderer->depthDummy);
	sg_destroy_image(globalRenderer->depthBuffer);
	sg_destroy_image(globalRenderer->normalBuffer);
	sg_destroy_image(globalRenderer->positionBuffer);
	sg_destroy_image(globalRenderer->noiseBuffer);
	sg_destroy_image(globalRenderer->prevFrameBuffer);
	OEDestroyViews(&globalRenderer->views);
	globalRenderer->renderTarget = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "render_target"
	});
	globalRenderer->postTarget = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "post_target"
	});
	globalRenderer->postTargetPong = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "post_target_p"
	});
	globalRenderer->depthDummy = sg_make_image(&(sg_image_desc){
		.usage.depth_stencil_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_DEPTH,
		.sample_count = 1,
		.label = "depthd_image"
	});
	globalRenderer->depthBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "depth_image"
	});
	globalRenderer->normalBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "normal_image"
	});
	globalRenderer->positionBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "position_image"
	});
	globalRenderer->noiseBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "noise_image"
	});
	globalRenderer->prevFrameBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = w, 
    	.height = h, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "previous_frame"
	});

	/*Color views init*/
	globalRenderer->views.cRenderTarget = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->renderTarget});
	globalRenderer->views.cDepthBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->depthBuffer});
	globalRenderer->views.cNormalBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->normalBuffer});
	globalRenderer->views.cPositionBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->positionBuffer});
	globalRenderer->views.cNoiseBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->noiseBuffer});
	globalRenderer->views.cPrevFrameBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->prevFrameBuffer});
	globalRenderer->views.cPostTarget = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->postTarget});
	globalRenderer->views.cPostTargetPong = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->postTargetPong});

	/*Texture views init*/
	globalRenderer->views.tRenderTarget = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->renderTarget});
	globalRenderer->views.tDepthBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->depthBuffer});
	globalRenderer->views.tNormalBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->normalBuffer});
	globalRenderer->views.tPositionBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->positionBuffer});
	globalRenderer->views.tNoiseBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->noiseBuffer});
	globalRenderer->views.tPrevFrameBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->prevFrameBuffer});
	globalRenderer->views.tPostTarget = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->postTarget});
	globalRenderer->views.tPostTargetPong = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->postTargetPong});
	sg_view depthDummyView = sg_make_view(&(sg_view_desc){
				.depth_stencil_attachment.image=globalRenderer->depthDummy});
	globalRenderer->renderTargetAtt = (sg_attachments){
		.colors[0] = globalRenderer->views.cRenderTarget,
		.colors[1] = globalRenderer->views.cDepthBuffer,
		.colors[2] = globalRenderer->views.cNormalBuffer,
		.colors[3] = globalRenderer->views.cPositionBuffer,
		.colors[4] = globalRenderer->views.cNoiseBuffer,
		.colors[5] = globalRenderer->views.cPrevFrameBuffer,
		.depth_stencil = depthDummyView};
	
	globalRenderer->postTargetAtt = (sg_attachments){
		.colors[0] = globalRenderer->views.cPostTarget,
		.depth_stencil = depthDummyView};
	globalRenderer->postTargetAttPong = (sg_attachments){
		.colors[0] = globalRenderer->views.cPostTargetPong,
		.depth_stencil = depthDummyView};
	globalRenderer->prevFrameTarg = (sg_attachments){
		.colors[0] = globalRenderer->views.cPrevFrameBuffer,
		.depth_stencil = depthDummyView};
	globalRenderer->window->renderWidth = w;
	globalRenderer->window->renderHeight = h;
}

_OE_COLD void OEForceGraphicsSetting(int flag) {
	if(!OECheckGraphicFlag(flag)) {
		WLOG(WARN, "Invalid Graphics Setting Flag!");
		return;
	}
	if(flag>OE_LOW_GRAPHICS) {
		OESetDefaultShader(globalRenderer->cubeShader);
		globalRenderer->graphicsSetting = flag;
	} else {
		OESetDefaultShader(globalRenderer->lowDefCubeShader);
		globalRenderer->graphicsSetting = flag;
	}
}

#ifdef OE_VULKAN
_OE_COLD void OEInitVKImages() {

	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
	//VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

	OEVKData *vk = globalRenderer->window->VK;

	VkSurfaceCapabilitiesKHR surfCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physDevice, vk->images.surface, &surfCaps);

	vk->images.swapInfo = (VkSwapchainCreateInfoKHR){
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vk->images.surface,
		.minImageCount = OEVKGetMinImageCount(&surfCaps),
		.imageFormat = format,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = {
			.width = surfCaps.currentExtent.width, 
			.height = surfCaps.currentExtent.height,
		},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE
	};
	WASSERT(vkCreateSwapchainKHR(vk->device, &vk->images.swapInfo, 
				NULL, &vk->images.swapchain)==VK_SUCCESS, "Failed to create swapchain!");

	uint32_t images;
	vkGetSwapchainImagesKHR(vk->device, vk->images.swapchain, &images, NULL);
	vk->images.images = calloc(images, sizeof(VkImage));
	vk->images.views = calloc(images, sizeof(VkImageView));
	vk->images.viewInfo = calloc(images, sizeof(VkImageViewCreateInfo));
	vkGetSwapchainImagesKHR(vk->device, vk->images.swapchain, &images, vk->images.images);
	int i;
	for(i=0;i<images;i++) {
		vk->images.viewInfo[i] = (VkImageViewCreateInfo){
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vk->images.images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = 1,
				.layerCount = 1
			}
		};
		WASSERT(vkCreateImageView(vk->device, &vk->images.viewInfo[i], 
					NULL, &vk->images.views[i])==VK_SUCCESS, "Failed to create view image!");
	} 
	vk->images.totalImages = images;

	vk->images.depthImageInfo = (VkImageCreateInfo){
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		.extent = {
			.width = surfCaps.currentExtent.width, 
			.height = surfCaps.currentExtent.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	WASSERT(vkCreateImage(vk->device, &vk->images.depthImageInfo, 
				NULL, &vk->images.depthImage)==VK_SUCCESS, "Failed to create depth image!");

	VkMemoryRequirements memReq;
	VkPhysicalDeviceMemoryProperties memProps;
	VkMemoryAllocateInfo allocInfo;

	vkGetImageMemoryRequirements(vk->device, vk->images.depthImage, &memReq);
	vkGetPhysicalDeviceMemoryProperties(vk->physDevice, &memProps);

	uint32_t mIndex = UINT32_MAX;
	for(i=0;i<memProps.memoryTypeCount;i++) {
		if((memReq.memoryTypeBits&(1<<i))&&
				(memProps.memoryTypes[i].propertyFlags&VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
			mIndex = i;
			break;
		}
	}
	WASSERT(mIndex!=UINT32_MAX, "Invalid requested memory index for depth image!");

	allocInfo = (VkMemoryAllocateInfo){
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReq.size,
		.memoryTypeIndex = mIndex
	};
	WASSERT(vkAllocateMemory(vk->device, &allocInfo,
				NULL, &vk->images.depthMem)==VK_SUCCESS, 
			"Failed to allocate memory for depth image!");
	WASSERT(vkBindImageMemory(vk->device, vk->images.depthImage,
				vk->images.depthMem, 0)==VK_SUCCESS, "Failed to bind depth image memory!");

	vk->images.depthImageViewInfo = (VkImageViewCreateInfo){
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = vk->images.depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	WASSERT(vkCreateImageView(vk->device, &vk->images.depthImageViewInfo,
				NULL, &vk->images.depthImageView)==VK_SUCCESS, 
				"Failed to create depth view!");
}

_OE_COLD void OEVKInitSemaphores() {
	OEVKData *vk = globalRenderer->window->VK;
	vk->images.renderSema = calloc(vk->images.totalImages, sizeof(VkSemaphore));
	vk->images.presentSema = calloc(vk->images.totalImages, sizeof(VkSemaphore));
	VkSemaphoreCreateInfo createInfo = (VkSemaphoreCreateInfo){
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};	
	int i;
	for(i=0;i<vk->images.totalImages;i++) {
		WASSERT(vkCreateSemaphore(vk->device, &createInfo,
					NULL, &vk->images.renderSema[i])==VK_SUCCESS, "Failed to create renderSema!");
		WASSERT(vkCreateSemaphore(vk->device, &createInfo, 
				NULL, &vk->images.presentSema[i])==VK_SUCCESS, "Failed to create presentSema!");
	}
	vk->images.semaSlotIndex = 0;
}

_OE_HOT void OEReinitVKImages() {
	OEVKData *vk = globalRenderer->window->VK;
	vkDeviceWaitIdle(vk->device);
	uint32_t iCount = 0;
	vkGetSwapchainImagesKHR(vk->device, vk->images.swapchain, &iCount, NULL);
	int i;
	for(i=0;i<iCount;i++) vkDestroyImageView(vk->device, vk->images.views[i], NULL);
	free(vk->images.views);
	free(vk->images.images);
	vkDestroySwapchainKHR(vk->device, vk->images.swapchain, NULL);
	vkDestroyImageView(vk->device, vk->images.depthImageView, NULL);
	vkDestroyImage(vk->device, vk->images.depthImage, NULL);
	vkFreeMemory(vk->device, vk->images.depthMem, NULL);
	for(i=0;i<iCount;i++) {
		vkDestroySemaphore(vk->device, vk->images.renderSema[i], NULL);
		vkDestroySemaphore(vk->device, vk->images.presentSema[i], NULL);
	}
	free(vk->images.renderSema);
	free(vk->images.presentSema);
	OEInitVKImages();
	OEVKInitSemaphores();
}

#endif

unsigned int OEIsVulkan() {return globalRenderer->isVulkan;}

_OE_COLD void OEInitRenderer(int width, int height, char *title, enum CamType camType) {

/*
 * GlobalRenderer & OpenGL/SDL setup
 * */

	globalRenderer = calloc(1, sizeof(struct renderer));
	globalRenderer->window = calloc(1, sizeof(Window));
	globalRenderer->window->width = width;
	globalRenderer->window->height = height;
	globalRenderer->window->renderWidth = width;
	globalRenderer->window->renderHeight = height;
	globalRenderer->window->title = strdup(title);
	globalRenderer->window->running = 1;
	globalRenderer->gameController = NULL;
	globalRenderer->tick = 0.0f;
	globalRenderer->debug = 0;
	globalRenderer->postPassSize = 0;
	globalRenderer->keyPressed = 0;
	globalRenderer->mousePressed = 0;
	globalRenderer->graphicsSetting = OE_HIGH_GRAPHICS;
	globalRenderer->legacy = 0;
	globalRenderer->imgui.ioptr = NULL;
	globalRenderer->window->cursor = NULL;
	globalRenderer->frame = 0;
	globalRenderer->disablesdtx = 0;
	globalRenderer->iBatchCap = OBJSTEP;
	globalRenderer->iBatchSize = 0;
	globalRenderer->iBatches = calloc(globalRenderer->iBatchCap, sizeof(OEInstanceBatch));
	char *os = OEGETOS();
	char *osclass = OEGETOSCLASS();
	globalRenderer->OSInfo = calloc(strlen(os)+strlen(osclass)+128, sizeof(char));
	snprintf(globalRenderer->OSInfo, sizeof(char)*(strlen(os)+strlen(osclass)+128), "%s: %s", osclass, os);

	int pp;
	for(pp=0;pp<MAXPOSTPASS;pp++) {
		globalRenderer->postPasses[pp] = (PostPass){0};
		globalRenderer->postPasses[pp].ID = NULL;
	}

	if(OSCLASS!=OS_WINDOWS) {
		setenv("MESA_GL_VERSION_OVERRIDE", "4.1", 0);
	}
	
	WASSERT(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER)>=0,
			"ERROR:: Failed to init SDL!");
#ifndef OE_VULKAN
	globalRenderer->isVulkan = 0;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	

	SDL_GL_LoadLibrary(NULL);

	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");

	globalRenderer->window->window = SDL_CreateWindow(
			globalRenderer->window->title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			globalRenderer->window->width, globalRenderer->window->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	globalRenderer->window->gl_context = 
		SDL_GL_CreateContext(globalRenderer->window->window);

	if(globalRenderer->window->gl_context==NULL||
			!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		/*fallback to gl 3.3*/
		OEGLFallbackInit();
	}

	SDL_GL_SetSwapInterval(1); /*VSYNC=on*/
	//SDL_GL_SetSwapInterval(0); /*VSYNC=off*/
 	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
	glFrontFace(GL_CCW);

	int w,h;
	SDL_GetWindowSize(globalRenderer->window->window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	WLOG(INFO_OS, globalRenderer->OSInfo);
	WLOG(INFO_VENDOR, glGetString(GL_VENDOR));
	WLOG(INFO_GPU, glGetString(GL_RENDERER));
	WLOG(INFO_DRIVER_VERSION, glGetString(GL_VERSION));
#else 
	globalRenderer->isVulkan = 1;
	globalRenderer->window->VK = calloc(1, sizeof(OEVKData));
	SDL_Vulkan_LoadLibrary(NULL);
	
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAMDECK, "1");

	globalRenderer->window->window = SDL_CreateWindow(
			globalRenderer->window->title, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			globalRenderer->window->width, globalRenderer->window->height,
			SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	uint32_t extCount;
	SDL_Vulkan_GetInstanceExtensions(globalRenderer->window->window, &extCount, NULL);
	const char **extNames = calloc(extCount+2, sizeof(const char *));
	SDL_Vulkan_GetInstanceExtensions(globalRenderer->window->window, &extCount, extNames);
	extNames[extCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	extNames[extCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;

	/*VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = OEVKDebugCallback,
		.pUserData = NULL
	};

	const char *validationLayers[] = {"VK_LAYER_MESA_device_select"};*/

	globalRenderer->window->VK->appInfo = (VkApplicationInfo){
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = globalRenderer->window->title,
		.pEngineName = "Obliviengine",
		.applicationVersion = VK_MAKE_VERSION(0, 0, 0),
		.engineVersion = VK_MAKE_VERSION(0, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};
	globalRenderer->window->VK->createInfo = (VkInstanceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &globalRenderer->window->VK->appInfo,
		.enabledExtensionCount = extCount,
		.ppEnabledExtensionNames = extNames,
		.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
		/*.enabledLayerCount = 1,
		.ppEnabledLayerNames = validationLayers,
		.pNext = &debugInfo*/
	};

	WASSERT(vkCreateInstance(&globalRenderer->window->VK->createInfo,
				NULL, &globalRenderer->window->VK->instance)==VK_SUCCESS, 
			"Failed to init Vulkan instance!");

	/*PFN_vkCreateDebugUtilsMessengerEXT debugMessenger =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(globalRenderer->window->VK->instance,
				"vkCreateDebugUtilsMessengerEXT");
	if(debugMessenger) {
		debugMessenger(globalRenderer->window->VK->instance,
				&debugInfo, NULL, &globalRenderer->window->VK->debugMessenger);
	}*/

	SDL_Vulkan_CreateSurface(globalRenderer->window->window,
			globalRenderer->window->VK->instance, &globalRenderer->window->VK->images.surface);

	uint32_t GPUCount = 0;
	vkEnumeratePhysicalDevices(globalRenderer->window->VK->instance, &GPUCount, NULL);
	WASSERT(GPUCount>0, "No GPU devices found!");
	char *gbuf = calloc(128, sizeof(char));
	snprintf(gbuf, sizeof(char)*128, "GPU Count: %d", GPUCount);
	WLOG(VK_INFO, gbuf);
	free(gbuf); gbuf = NULL;

	VkPhysicalDevice *devices = calloc(GPUCount, sizeof(VkPhysicalDevice));	
	vkEnumeratePhysicalDevices(globalRenderer->window->VK->instance, &GPUCount, devices);
	globalRenderer->window->VK->physDevice = devices[0];
	
	uint32_t qFamCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(globalRenderer->window->VK->physDevice, &qFamCount, NULL);
	VkQueueFamilyProperties *qFamProps = calloc(qFamCount, sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(globalRenderer->window->VK->physDevice, &qFamCount, qFamProps);
	int qi;
	for(qi=0;qi<qFamCount;qi++) {
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(globalRenderer->window->VK->physDevice, qi,
				globalRenderer->window->VK->images.surface, &presentSupport);
		if((qFamProps[qi].queueFlags&VK_QUEUE_GRAPHICS_BIT)&&presentSupport==VK_TRUE) {
			globalRenderer->window->VK->queueFamIndex = qi;
			break;
		}
	}
	free(qFamProps);

	globalRenderer->window->VK->queuePrio = 1.0f;
	globalRenderer->window->VK->queueInfo = (VkDeviceQueueCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = globalRenderer->window->VK->queueFamIndex,
		.queueCount = 1,
		.pQueuePriorities = &globalRenderer->window->VK->queuePrio 
	};

	const char *dExt[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME};

	globalRenderer->window->VK->descriptorBuffFeatures = (VkPhysicalDeviceDescriptorBufferFeaturesEXT){
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
		.descriptorBuffer = VK_TRUE
	};
	globalRenderer->window->VK->xdsFeatures = (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT){
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = &globalRenderer->window->VK->descriptorBuffFeatures,
		.extendedDynamicState = VK_TRUE
	};
	globalRenderer->window->VK->features12 = (VkPhysicalDeviceVulkan12Features){
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &globalRenderer->window->VK->xdsFeatures,
		.bufferDeviceAddress = VK_TRUE
	};
	globalRenderer->window->VK->features13 = (VkPhysicalDeviceVulkan13Features){
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &globalRenderer->window->VK->features12,
		.dynamicRendering = VK_TRUE,
		.synchronization2 = VK_TRUE
	};

	VkPhysicalDeviceFeatures2 supports;
	supports.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	supports.pNext = NULL;
	vkGetPhysicalDeviceFeatures2(globalRenderer->window->VK->physDevice, &supports);
	globalRenderer->window->VK->features2 = (VkPhysicalDeviceFeatures2){
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &globalRenderer->window->VK->features13,
		.features.samplerAnisotropy = VK_TRUE,
		.features.dualSrcBlend = VK_TRUE
	};
	if(supports.features.textureCompressionBC)
		globalRenderer->window->VK->features2.features.textureCompressionBC = VK_TRUE;
	if(supports.features.textureCompressionETC2)
		globalRenderer->window->VK->features2.features.textureCompressionETC2 = VK_TRUE;
	if(supports.features.textureCompressionASTC_LDR)
		globalRenderer->window->VK->features2.features.textureCompressionASTC_LDR = VK_TRUE;

	globalRenderer->window->VK->deviceInfo = (VkDeviceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &globalRenderer->window->VK->features2,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &globalRenderer->window->VK->queueInfo,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = dExt,
	};

	WASSERT(vkCreateDevice(globalRenderer->window->VK->physDevice, 
			&globalRenderer->window->VK->deviceInfo, NULL,
			&globalRenderer->window->VK->device)==VK_SUCCESS, 
			"Failed to create VK device!");

	vkGetDeviceQueue(globalRenderer->window->VK->device,
			globalRenderer->window->VK->queueFamIndex, 0,
			&globalRenderer->window->VK->queue);

	OEInitVKImages();
	OEVKInitSemaphores();
#endif
/*
 * Sokol setup
 * */

	sg_setup(&(sg_desc){
			.environment = OEGetEnv(),
			.logger.func = slog_func,
#ifdef OE_VULKAN
			.vulkan.stream_staging_buffer_size = 64*1024*1024
#endif
	});

	if(!globalRenderer->legacy) {
		/*sdtx_setup(&(sdtx_desc_t){
			.fonts = {[1]  = sdtx_font_oric()},
			.logger.func = slog_func});*/
	} else {
		/*TODO: sdtx for legacy GL 3.3*/
	}


	if(!OEDumpSupportedPixelFormats())
			WLOG(ERROR, "One or more required pixel formats are not supported on your system!");

/*
 * Cimgui Setup
 * */

#ifndef OE_VULKAN
	igCreateContext(NULL);
	globalRenderer->imgui.ioptr = igGetIO();
	ImGuiIO *iotmp = globalRenderer->imgui.ioptr;

	ImGui_ImplSDL2_InitForOpenGL(globalRenderer->window->window, 
			globalRenderer->window->gl_context);
	globalRenderer->igStat = 0;
	if(!globalRenderer->legacy)
		globalRenderer->igStat = (int)ImGui_ImplOpenGL3_Init("#version 410");
	else globalRenderer->igStat = (int)ImGui_ImplOpenGL3_Init("#version 330");
#else
	simgui_desc_t desc = {.logger.func = slog_func};
	simgui_setup(&desc);
	ImGui_ImplSDL2_InitForVulkan(globalRenderer->window->window);
	globalRenderer->igStat = 1;
#endif

	if(!globalRenderer->igStat) 
		WLOG(IMGUI_INFO, "Failed to initialize Imgui!");

	igStyleColorsDark(NULL);
/*
 * setup Render Target & Depth Buffer
 * */

	globalRenderer->renderTarget = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "render_target"
	});

	globalRenderer->postTarget = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "post_target"
	});
	globalRenderer->postTargetPong = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
		.width = globalRenderer->window->width,
		.height = globalRenderer->window->height,
		.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.label = "post_target_p"
	});
	globalRenderer->depthDummy = sg_make_image(&(sg_image_desc){
		.usage.depth_stencil_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_DEPTH,
		.sample_count = 1,
		.label = "depthd_image"
	});
	globalRenderer->depthBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "depth_image"
	});
	globalRenderer->normalBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "normal_image"
	});
	globalRenderer->positionBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "position_image"
	});
	globalRenderer->noiseBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "noise_image"
	});
	globalRenderer->prevFrameBuffer = sg_make_image(&(sg_image_desc){
		.usage.color_attachment = true,
    	.width = globalRenderer->window->width, 
    	.height = globalRenderer->window->height, 
    	.pixel_format = SG_PIXELFORMAT_RGBA32F,
		.sample_count = 1,
		.label = "previous_frame"
	});

	/*Color views init*/
	globalRenderer->views.cRenderTarget = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->renderTarget});
	globalRenderer->views.cDepthBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->depthBuffer});
	globalRenderer->views.cNormalBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->normalBuffer});
	globalRenderer->views.cPositionBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->positionBuffer});
	globalRenderer->views.cNoiseBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->noiseBuffer});
	globalRenderer->views.cPrevFrameBuffer = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->prevFrameBuffer});
	globalRenderer->views.cPostTarget = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->postTarget});
	globalRenderer->views.cPostTargetPong = 
		sg_make_view(&(sg_view_desc){.color_attachment.image=globalRenderer->postTargetPong});


	/*Texture views init*/
	globalRenderer->views.tRenderTarget = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->renderTarget});
	globalRenderer->views.tDepthBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->depthBuffer});
	globalRenderer->views.tNormalBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->normalBuffer});
	globalRenderer->views.tPositionBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->positionBuffer});
	globalRenderer->views.tNoiseBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->noiseBuffer});
	globalRenderer->views.tPrevFrameBuffer = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->prevFrameBuffer});
	globalRenderer->views.tPostTarget = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->postTarget});
	globalRenderer->views.tPostTargetPong = 
		sg_make_view(&(sg_view_desc){.texture.image=globalRenderer->postTargetPong});

	OECheckOEViews(&globalRenderer->views);

	sg_view depthDummyView = sg_make_view(&(sg_view_desc){
				.depth_stencil_attachment.image=globalRenderer->depthDummy});

	globalRenderer->renderTargetAtt = (sg_attachments){
		.colors[0] = globalRenderer->views.cRenderTarget,
		.colors[1] = globalRenderer->views.cDepthBuffer,
		.colors[2] = globalRenderer->views.cNormalBuffer,
		.colors[3] = globalRenderer->views.cPositionBuffer,
		.colors[4] = globalRenderer->views.cNoiseBuffer,
		.colors[5] = globalRenderer->views.cPrevFrameBuffer,
		.depth_stencil = depthDummyView};
	
	globalRenderer->postTargetAtt = (sg_attachments){
		.colors[0] = globalRenderer->views.cPostTarget,
		.depth_stencil = depthDummyView};
	globalRenderer->postTargetAttPong = (sg_attachments){
		.colors[0] = globalRenderer->views.cPostTargetPong,
		.depth_stencil = depthDummyView};
	globalRenderer->prevFrameTarg = (sg_attachments){
		.colors[0] = globalRenderer->views.cPrevFrameBuffer,
		.depth_stencil = depthDummyView};

	globalRenderer->sampler = sg_make_sampler(&(sg_sampler_desc){
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
	/*
	 * I know this looks sketchy to use "4.1" shaders on 3.3 as well, 
	 * but we actually don't use any post 3.3 functions, so they work!
	 * */
	//if(!globalRenderer->legacy) {
		globalRenderer->renderTargetShade = sg_make_shader(quad_shader_desc(sg_query_backend()));
		globalRenderer->cubeShader = sg_make_shader(simple_shader_desc(sg_query_backend()));
		globalRenderer->lowDefCubeShader = sg_make_shader(simple_low_shader_desc(sg_query_backend()));
		if(globalRenderer->graphicsSetting<OE_HIGH_GRAPHICS)
			OESetDefaultShader(globalRenderer->lowDefCubeShader);
		else OESetDefaultShader(globalRenderer->cubeShader);

		globalRenderer->ppshaders.fxaa = sg_make_shader(OEFXAA_shader_desc(sg_query_backend()));
		globalRenderer->ppshaders.ssao = sg_make_shader(OESSAO_shader_desc(sg_query_backend()));
		globalRenderer->ppshaders.bloom = sg_make_shader(OEBQuad_shader_desc(sg_query_backend()));
		sg_pipeline_desc fxaapd = OEGetQuadPipeline(globalRenderer->ppshaders.fxaa, "fxaa");
		globalRenderer->ppshaders.fxaap = sg_make_pipeline(&fxaapd);
		sg_pipeline_desc ssaopd = OEGetQuadPipeline(globalRenderer->ppshaders.ssao, "ssao");
		globalRenderer->ppshaders.ssaop = sg_make_pipeline(&ssaopd);
		sg_pipeline_desc bloompd = OEGetQuadPipeline(globalRenderer->ppshaders.bloom, "bloom");
		globalRenderer->ppshaders.bloomp = sg_make_pipeline(&bloompd);
		globalRenderer->ppshaders.ssgi = sg_make_shader(OESSGI_shader_desc(sg_query_backend()));
		sg_pipeline_desc ssgipd = OEGetQuadPipeline(globalRenderer->ppshaders.ssgi, "ssgi");
		globalRenderer->ppshaders.ssgip = sg_make_pipeline(&ssgipd);
		globalRenderer->ppshaders.dnoise = sg_make_shader(OEDNOISE_shader_desc(sg_query_backend()));
		sg_pipeline_desc dnoisepd = OEGetQuadPipeline(globalRenderer->ppshaders.dnoise, "dnoise");
		globalRenderer->ppshaders.dnoisep = sg_make_pipeline(&dnoisepd);
	//} else {
		/*TODO: Build GL 330 shaders*/
	//	WLOG(WARN, "Using legacy fallback shaders");
	//}

	sg_pipeline_desc rtp = OEGetQuadPipeline(globalRenderer->renderTargetShade, 
			"render_target_pipe");

	globalRenderer->renderTargetPipe = sg_make_pipeline(&rtp);

	globalRenderer->renderTargetBuff = sg_make_buffer(&(sg_buffer_desc) {
				.data = SG_RANGE(quadVertices),
				.label = "quad_verts"
			});

	sg_shader instShade = sg_make_shader(simpleInst_shader_desc(sg_query_backend()));
	sg_pipeline_desc ibpd = OEGetInstancingPipe(instShade, "instPipe");
	globalRenderer->iBatchPipe = sg_make_pipeline(&ibpd);
	globalRenderer->originalIBatchShader = instShade;

/*
 * Init objects
 * */
	globalRenderer->objCap = OBJSTEP;
	globalRenderer->objSize = 0;
	globalRenderer->objects = calloc(globalRenderer->objCap, sizeof(Object));
	
	initBaseObjects();

	static const uint32_t white = OE_WHITEP;
	sg_image_desc img_desc = {
		.width = 1,
		.height = 1,
		.pixel_format = SG_PIXELFORMAT_RGBA8,
		.data.mip_levels[0] = SG_RANGE(white),
    	.label = "defTexture"
	};

	globalRenderer->defTexture = sg_make_view(&(sg_view_desc){
			.texture.image=sg_make_image(&img_desc)});

/*
 * Init Camera
 * */

	globalRenderer->cam.oScale = 3.0f;
	float oScale = globalRenderer->cam.oScale;

	mat4x4_identity(globalRenderer->cam.model);
	mat4x4_identity(globalRenderer->cam.view);
	mat4x4_identity(globalRenderer->cam.proj);
	mat4x4_identity(globalRenderer->cam.mvp);
	mat4x4_identity(globalRenderer->cam.rotation);

	vec3_dup(globalRenderer->cam.up, (vec3){0.0f, 1.0f, 0.0f});
	globalRenderer->cam.fov = DEG2RAD(80.0f);

	float aspect = (float)globalRenderer->window->width / (float)globalRenderer->window->height;
	globalRenderer->cam.aspect = aspect;
	Vec3 tmp;

	globalRenderer->camType = camType;
	switch(camType) {
		case ISOMETRIC: {
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

			vec3_dup(globalRenderer->cam.target, (vec3){0.0f,0.0f,0.0f});
			mat4x4_look_at(globalRenderer->cam.view,
			               globalRenderer->cam.position,
			               globalRenderer->cam.target,
			               globalRenderer->cam.up);
#ifndef OE_VULKAN
			mat4x4_ortho(globalRenderer->cam.proj, 
			             -oScale * aspect, oScale * aspect, 
			             -oScale, oScale, 
			             0.1f, 80.0f);
#else
			mat4x4_ortho_vulkan(globalRenderer->cam.proj, 
			             -oScale * aspect, oScale * aspect, 
			             -oScale, oScale, 
			             0.1f, 80.0f);
#endif

			mat4x4_mul(globalRenderer->cam.mvp, globalRenderer->cam.proj, globalRenderer->cam.view);
			mat4x4_mul(globalRenderer->cam.mvp, globalRenderer->cam.mvp, globalRenderer->cam.model);

			break;
		}
		case PERSPECTIVE: {
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

			mat4x4_mul(globalRenderer->cam.mvp, globalRenderer->cam.proj, globalRenderer->cam.view);
			mat4x4_mul(globalRenderer->cam.mvp, globalRenderer->cam.mvp, globalRenderer->cam.model);

			OEComputeRotationMatrix(globalRenderer->cam.rotation,
				globalRenderer->cam.front, 
				globalRenderer->cam.up);

			break;
		}
	};

	/*
	 * Init Lua Scripting
	 * */
	OEInitLua(&globalRenderer->luaData);
	int i;
	for(i=0;i<globalRenderer->objSize;i++) globalRenderer->objects[i].script.filePath = NULL;

	/*
	 * Init OEUI
	 * */
	OEUIInit(&globalRenderer->oeuiData, __FILE__);
}

_OE_HOT void OEUpdateViewMat() {
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
#ifndef OE_VULKAN
		mat4x4_ortho(globalRenderer->cam.proj, 
				-oScale * aspect, oScale * aspect, 
				-oScale, oScale, 0.1f, 100.0f);
#else
		mat4x4_ortho_vulkan(globalRenderer->cam.proj, 
				-oScale * aspect, oScale * aspect, 
				-oScale, oScale, 0.1f, 100.0f);
#endif
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
				if(cam->position[0]<0.1f) cam->position[0]=0.1f;
				if(cam->position[1]<0.1f) cam->position[1]=0.1f;
				if(cam->position[1]<0.1f) cam->position[1]=0.1f;
				break;
			}
			vec3_scale(tmp, cam->up, -len);
			vec3_add(cam->position, cam->position, tmp);
			break;
	};

	OEUpdateViewMat();
}

void OECamSet(vec3 pos) {
	Camera *cam = &globalRenderer->cam;
	vec3_dup(cam->position, pos);
	OEUpdateViewMat();
}

_OE_PURE Camera *OEGetCamera() {
	return &globalRenderer->cam;
}

_OE_PURE Vec3 OEGetCamPos() {
	return (Vec3){globalRenderer->cam.position[0],
				globalRenderer->cam.position[1],
				globalRenderer->cam.position[2]};
}

_OE_PURE SDL_Event OEGetEvent() {
	return globalRenderer->event;
}

_OE_PURE int OEIsKeyPressed() {
	return globalRenderer->keyPressed;
}

_OE_PURE int OEIsKeyRepeating() {
	return globalRenderer->wasKeyPressed;
}

_OE_PURE int OEGetKeySym() {
	return globalRenderer->lastKey;
}

void OEGetMousePos(int *_OE_RESTRICT x, int *_OE_RESTRICT y) {
	SDL_GetMouseState(x,y);
}

_OE_PURE Mouse OEGetMouse() {
	return globalRenderer->mouse;
}

_OE_PURE int OEIsMousePressed() {
	return globalRenderer->mousePressed;
}

_OE_PURE int OEIsMouseRepeating() {
	return globalRenderer->wasMousePressed;
}

_OE_PURE SDL_MouseButtonEvent *OEGetMouseEvent() {
	return &globalRenderer->mouseEvent;
}

void OESetWindowFullscreen() {
	SDL_SetWindowFullscreen(globalRenderer->window->window, SDL_WINDOW_FULLSCREEN);
}

void OESetWindowFullscreenDesktop() {
	SDL_SetWindowFullscreen(globalRenderer->window->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void OESetWindowBorderless() {
	SDL_SetWindowBordered(globalRenderer->window->window, SDL_FALSE);
}

void OESetWindowBordered() {
	SDL_SetWindowBordered(globalRenderer->window->window, SDL_TRUE);
}

void OESetWindowUsableScreen() {
	SDL_Rect ss;
	if(!SDL_GetDisplayUsableBounds(
				SDL_GetWindowDisplayIndex(globalRenderer->window->window), &ss)) {
		globalRenderer->window->width = ss.w;
		globalRenderer->window->height = ss.h;
		SDL_SetWindowPosition(globalRenderer->window->window, 0,0);
		SDL_SetWindowSize(globalRenderer->window->window,
				globalRenderer->window->width,
				globalRenderer->window->height);
		char buf[1024];
		snprintf(buf, sizeof(buf), "Set window size to: %d, %d", 
				globalRenderer->window->width,
				globalRenderer->window->height);
		WLOG(INFO, buf);
	}
}

void OESetWindowDisplayMode(int flag) {
	if(!OECheckScreenFlag(flag)) return;
	switch(flag) {
		case OE_USABLE_SCREEN_SPACE: OESetWindowUsableScreen();break;
		case OE_FULLSCREEN: OESetWindowFullscreen();break;
		case OE_FULLSCREEN_DESKTOP: OESetWindowFullscreenDesktop();break;
		case OE_BORDERLESS: OESetWindowBorderless();break;
		case OE_BORDERED: OESetWindowBordered();break;
	};
}

unsigned int OEGetMouseScrollUp() {
	return globalRenderer->mouseScrollUp;
}

unsigned int OEGetMouseScrollDown() {
	return globalRenderer->mouseScrollDown;
}

SDL_GameController *OEGetGameController() {
	return globalRenderer->gameController;
}

/*For mixed events use an SDL keyboard state*/
void OEPollEvents(EVENTFUNC event) {
	globalRenderer->wasKeyPressed = OEIsKeyPressed();
	globalRenderer->wasMousePressed = OEIsMousePressed();
	globalRenderer->mouseScrollUp = 0;
	globalRenderer->mouseScrollDown = 0;
	while(SDL_PollEvent(&globalRenderer->event)!=0) {
		ImGui_ImplSDL2_ProcessEvent(&globalRenderer->event);
		if(globalRenderer->event.type==SDL_CONTROLLERDEVICEADDED) {
			globalRenderer->gameController = 
				SDL_GameControllerOpen(globalRenderer->event.cdevice.which);
		}
		if(globalRenderer->event.type==SDL_QUIT) {
			globalRenderer->window->running = 0;
		}
		if(globalRenderer->event.type==SDL_KEYDOWN) {
			globalRenderer->keyPressed = 1;
			globalRenderer->lastKey = globalRenderer->event.key.keysym.sym;
		} else if(globalRenderer->event.type==SDL_KEYUP) {
			globalRenderer->keyPressed = 0;
		}
		if(globalRenderer->event.type==SDL_MOUSEBUTTONDOWN) {
			globalRenderer->mousePressed = 1;
			globalRenderer->mouseEvent = globalRenderer->event.button;
		} else if(globalRenderer->event.type==SDL_MOUSEBUTTONUP) {
			globalRenderer->mousePressed = 0; 
			globalRenderer->mouseEvent = globalRenderer->event.button;
		}

		if(globalRenderer->event.type==SDL_MOUSEWHEEL) {
			if(globalRenderer->event.wheel.y>0) {
				globalRenderer->mouseScrollUp = 1;
			} else if(globalRenderer->event.wheel.y<0) {
				globalRenderer->mouseScrollDown = 1;
			} 
		}
	}
	event();
}

_OE_PURE int OEGetFPS() {
	return (int)globalRenderer->fps;
}

_OE_PURE float OEGetFrameTime() {
	return globalRenderer->frameTime;
}

_OE_PURE unsigned int OEGetFrameSwap() {
	return globalRenderer->frame;
}

_OE_PURE float OEGetTick() {
	return globalRenderer->tick;
}

_OE_PURE SDL_Window *OEGetWindow() {
	return globalRenderer->window->window;
}

void OEGetWindowResolution(int *x, int *y) {
	*x = globalRenderer->window->width;
	*y = globalRenderer->window->height;
}

void OEGetRenderResolution(int *x, int *y) {
	*x = globalRenderer->window->renderWidth;
	*y = globalRenderer->window->renderHeight;
}

_OE_PURE OEUIData *OEGetOEUIData() {
	return &globalRenderer->oeuiData;
}

_OE_PURE char *OEQueryOSInfo() {
	return globalRenderer->OSInfo;
}

void OESetCursor(char *filePath) {
	int w, h, channels, i;
	unsigned char *image = stbi_load(filePath, &w, &h, &channels, 4);
	if(!image) return;

	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(image, w, h,
			32, w*4, SDL_PIXELFORMAT_RGBA32);
	if(!surface) {stbi_image_free(image);return;}

	SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0,0);
	if(cursor==NULL) return;
	SDL_SetCursor(cursor);
	SDL_ShowCursor(SDL_ENABLE);
	globalRenderer->window->cursor = cursor;

	SDL_FreeSurface(surface);
	stbi_image_free(image);
}

/*returns the ID*/
PostPass *OEAddPostPass(char *id, sg_pipeline pipe, UNILOADER loader) {
	if(globalRenderer->postPassSize<MAXPOSTPASS) {
		/*Even though we organize the passes after removal we need to check it*/
		int i;
		for(i=0;i<MAXPOSTPASS;i++) {
			if(globalRenderer->postPasses[i].ID==NULL) {
				globalRenderer->postPasses[i] = 
					(PostPass){0.0f, id, pipe, loader};
				globalRenderer->postPassSize++;
				return &globalRenderer->postPasses[i];
			}
		}
	} else WLOG(WARN, "Too many post passes, skipping.");
	return NULL;
}

void OERemovePostPass(char *id) {
	int i,j=-1;
	for(i=0;i<globalRenderer->postPassSize;i++) {
		if(globalRenderer->postPasses[i].ID!=NULL&&
				!strcmp(globalRenderer->postPasses[i].ID,id)) {j=i;break;}
	}
	if(j<0) return;
	for(i=j;i<globalRenderer->postPassSize-1;i++) globalRenderer->postPasses[i]=globalRenderer->postPasses[i+1];
	globalRenderer->postPasses[globalRenderer->postPassSize-1] = (PostPass){0};
	globalRenderer->postPassSize--;

}

float OEGetPostPassTime(char *ID) {
	if(ID==NULL) return 0.0f;
	int i;
	for(i=0;i<globalRenderer->postPassSize;i++) {
		if(!strcmp(globalRenderer->postPasses[i].ID, ID)) 
			return globalRenderer->postPasses[i].passTime;
	}
	return 0.0f;
}

void OEEnableFXAA() {
	OEAddPostPass(OEFXAA, globalRenderer->ppshaders.fxaap, (UNILOADER)applyFXAAUniforms);
}

void OEDisableFXAA() {
	OERemovePostPass(OEFXAA);
}

void OEUpdateBloomParams(float threshold, float strength) {
	globalRenderer->bloomParams.thresh = threshold;
	globalRenderer->bloomParams.strength = strength;
}

void OEEnableBloom(float threshold, float strength) {
	globalRenderer->bloomParams.resolution[0] = globalRenderer->window->width;
	globalRenderer->bloomParams.resolution[1] = globalRenderer->window->height;
	globalRenderer->bloomParams.thresh = threshold;
	globalRenderer->bloomParams.strength = strength;
	OEAddPostPass(OEBLOOM, globalRenderer->ppshaders.bloomp, (UNILOADER)applyBloomUniforms);
}

void OEDisableBloom() {
	OERemovePostPass(OEBLOOM);
}

void OEEnableSSAO() {
	OEAddPostPass(OESSAO, globalRenderer->ppshaders.ssaop, (UNILOADER)applySSAOUniforms);
}

void OEDisableSSAO() {
	OERemovePostPass(OESSAO);
}

void OEEnableSSGI(int rays, int steps) {
	globalRenderer->ssgiParams.RAYS = rays;
	globalRenderer->ssgiParams.STEPS = steps;
	OEAddPostPass(OESSGI, globalRenderer->ppshaders.ssgip, (UNILOADER)applySSGIUniforms);
	if(rays<=10||steps<=6) OEEnableDNOISE(); 
}

void OEDisableSSGI() {
	OERemovePostPass(OESSGI);
	if(globalRenderer->ssgiParams.RAYS<=10||
			globalRenderer->ssgiParams.STEPS<=6) OEDisableDNOISE();
}

void OEUpdateSSGIParams(int rays, int steps) {
	globalRenderer->ssgiParams.RAYS = rays;
	globalRenderer->ssgiParams.STEPS = steps;
}

void OEEnableDNOISE() {
	globalRenderer->deNoiseParams.resolution[0] = globalRenderer->window->width;
	globalRenderer->deNoiseParams.resolution[1] = globalRenderer->window->height;
	OEAddPostPass(OEDNOISE, globalRenderer->ppshaders.dnoisep, (UNILOADER)applyDnoiseUniforms);	
}

void OEDisableDNOISE() {
	OERemovePostPass(OEDNOISE);
}

_OE_HOT void OERenderFrame(RENDFUNC drawCall, RENDFUNC cimgui, RENDFUNC OEUI) {
	globalRenderer->frame_start = SDL_GetPerformanceCounter();
	int winW, winH;
	SDL_GetWindowSize(globalRenderer->window->window,
			&winW, &winH);
	if(winW!=globalRenderer->window->width||winH!=globalRenderer->window->height) {
		globalRenderer->window->width = winW;
		globalRenderer->window->height = winH;
#ifdef OE_VULKAN
		OEReinitVKImages();
#endif
	}
	
	OEGetMousePos(&globalRenderer->mouse.posx, &globalRenderer->mouse.posy);

#ifdef OE_VULKAN
	OEVKData *vk = globalRenderer->window->VK;
	WASSERT(vkAcquireNextImageKHR(vk->device, vk->images.swapchain, UINT64_MAX, 
			vk->images.presentSema[vk->images.semaSlotIndex], 
			VK_NULL_HANDLE, &vk->images.imageIndex)==VK_SUCCESS, 
			"Failed to swap VK images!");
#endif

	sg_pass_action pass_action = (sg_pass_action) {
       	.colors[0] = {
           	.load_action = SG_LOADACTION_CLEAR,
       		.clear_value = {0.0f,0.0f,0.0f,0.0f}
        },
		.depth = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = 1.0f
		}
    };
	sg_pass_action off_pass_action = (sg_pass_action) {
       	.colors = {
           	[0] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[1] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[2] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[3] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[4] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}},
			[5] = {.load_action = SG_LOADACTION_CLEAR,.clear_value = {0.0f,0.0f,0.0f,1.0f}}
        },
		.depth = {
			.load_action = SG_LOADACTION_CLEAR,
			.clear_value = 1.0f
		}
    };
	sg_pass_action post_pass_action = (sg_pass_action) {
		.colors[0].load_action = SG_LOADACTION_DONTCARE,
		.depth = {
			.load_action = SG_LOADACTION_DONTCARE,
			.clear_value = 1.0f
		}
	};

	/*Offscreen pass to the texture*/
    sg_begin_pass(&(sg_pass){ .action = off_pass_action,
			.attachments = globalRenderer->renderTargetAtt});
	
	drawCall();
	sg_end_pass();

	sg_view src = globalRenderer->views.tRenderTarget;
	sg_view final = src;

	/*Post-passes*/
	int i;
	for(i=0;i<globalRenderer->postPassSize;i++) {
		uint32_t srcimg = sg_query_view_image(src).id;
		sg_view dst = (srcimg==sg_query_view_image(globalRenderer->views.tRenderTarget).id||
				srcimg==sg_query_view_image(globalRenderer->views.tPostTargetPong).id)
				?globalRenderer->views.tPostTarget:globalRenderer->views.tPostTargetPong;
		sg_attachments dstAtt = (srcimg==sg_query_view_image(globalRenderer->views.tRenderTarget).id||
				srcimg==sg_query_view_image(globalRenderer->views.tPostTargetPong).id)
				?globalRenderer->postTargetAtt:globalRenderer->postTargetAttPong;

		/*GLuint query[2];
		glGenQueries(2, query);
		glQueryCounter(query[0], GL_TIMESTAMP);*/

		sg_begin_pass(&(sg_pass){ .action = post_pass_action,
				.attachments = dstAtt});

		sg_apply_pipeline(globalRenderer->postPasses[i].pipe);

		sg_apply_bindings(&(sg_bindings){
			.vertex_buffers[0] = globalRenderer->renderTargetBuff,
			.views[VIEW_OEquad_texture] = src,
			.views[1] = globalRenderer->views.tDepthBuffer,
			.views[2] = globalRenderer->views.tNormalBuffer,
			.views[3] = globalRenderer->views.tPositionBuffer,
			.views[4] = globalRenderer->views.tNoiseBuffer,
			.views[5] = globalRenderer->views.tPrevFrameBuffer,
			.samplers[SMP_OEquad_smp] = globalRenderer->sampler,
		});
		if(globalRenderer->postPasses[i].uniformBind!=NULL) 
			globalRenderer->postPasses[i].uniformBind();
		sg_draw(0,6,1);

		sg_end_pass();

		/*This isn't really accurate to the frametime, but still gives an idea.*/
		/*glQueryCounter(query[1], GL_TIMESTAMP);
		GLuint64 GPUstart, GPUend;
		glGetQueryObjectui64v(query[0], GL_QUERY_RESULT, &GPUstart);
		glGetQueryObjectui64v(query[1], GL_QUERY_RESULT, &GPUend);
		globalRenderer->postPasses[i].passTime = (float)(GPUend-GPUstart)/100000.0f;*/

		src = dst;
		final = src;
	}
	
	/*Get the previous frame into it's buffer
	 * Previous frame storage is only on higher end graphics*/
	if(globalRenderer->graphicsSetting>OE_LOW_GRAPHICS) {
		post_pass_action.depth.load_action = SG_LOADACTION_DONTCARE;
		sg_begin_pass(&(sg_pass){ .action = post_pass_action,
				.attachments = globalRenderer->prevFrameTarg});
		sg_apply_pipeline(globalRenderer->renderTargetPipe);
		sg_apply_bindings(&(sg_bindings){
			.vertex_buffers[0] = globalRenderer->renderTargetBuff,
			.views[0] = final,
			.samplers[0] = globalRenderer->sampler
		});
		sg_draw(0,6,1);
		sg_end_pass();
	}

	/*OEUI pass*/
	if(OEUI!=NULL) {
		sg_pass_action ui_pass_action = (sg_pass_action) {
			.colors[0].load_action = SG_LOADACTION_LOAD,
			.depth = {
				.load_action = SG_LOADACTION_LOAD,
				.clear_value = 1.0f
			}
		};
		uint32_t finalImg = sg_query_view_image(final).id;
		sg_attachments uiAtt;
		if(finalImg==sg_query_view_image(globalRenderer->views.tRenderTarget).id)
			uiAtt = globalRenderer->renderTargetAtt;
		else if(finalImg==sg_query_view_image(globalRenderer->views.tPostTarget).id)
			uiAtt = globalRenderer->postTargetAtt;
		else if(finalImg==sg_query_view_image(globalRenderer->views.tPostTargetPong).id)
			uiAtt = globalRenderer->postTargetAttPong;
		sg_begin_pass(&(sg_pass){ .action = ui_pass_action,
				.attachments = uiAtt});
		OEUI();
		sg_end_pass();
	}

	/*On-Screen main pass*/
	sg_begin_pass(&(sg_pass){ .action = pass_action,
			.swapchain = OEGetSwapChain()});

	sg_apply_pipeline(globalRenderer->renderTargetPipe);
	sg_apply_bindings(&(sg_bindings){
		.vertex_buffers[0] = globalRenderer->renderTargetBuff,
		.views[0] = final,
		.samplers[0] = globalRenderer->sampler
	});
	sg_draw(0,6,1);

#ifndef OE_VULKAN /*sdtx doesn't work on Vulkan yet?*/
	if(globalRenderer->debug&&!globalRenderer->legacy&&!globalRenderer->disablesdtx) {
		sdtx_canvas(globalRenderer->window->width * 0.5f, 
				globalRenderer->window->height * 0.5f);
    	sdtx_origin(1.0f, 1.0f);
		sdtx_font(1);
		sdtx_color3b(0xf4, 0x43, 0x36);
		Camera *cam = &globalRenderer->cam;
		sdtx_printf("OS: %s\nFPS: %d\nFrameTime: %f\nCamPos: %f, %f, %f\n\n"
					"Resolution: %d,%d\nGPU: %s\nDriver: %s", 
				globalRenderer->OSInfo,
				(int)globalRenderer->fps, globalRenderer->frameTime,
				cam->position[0], cam->position[1], cam->position[2],
				globalRenderer->window->width, globalRenderer->window->height,
				glGetString(GL_RENDERER), glGetString(GL_VERSION));
		sdtx_draw();
	}
#endif

	if(cimgui!=NULL&&globalRenderer->igStat) {
#ifndef OE_VULKAN
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		igNewFrame();
		cimgui();
		igRender();
		ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
#else
		simgui_new_frame(&(simgui_frame_desc_t){
				globalRenderer->window->width, globalRenderer->window->height,
				globalRenderer->frameTime, 1.0f});
		cimgui();
		simgui_render();
#endif
	}

	sg_end_pass();

	sg_commit();

#ifndef OE_VULKAN
	SDL_GL_SwapWindow(globalRenderer->window->window);
	if(globalRenderer->window->cursor!=NULL)
		SDL_SetCursor(globalRenderer->window->cursor);
#else 
	vk->images.presentInfo = (VkPresentInfoKHR){
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &vk->images.renderSema[vk->images.imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &vk->images.swapchain,
		.pImageIndices = &vk->images.imageIndex
	};	
	vkQueuePresentKHR(vk->queue, &vk->images.presentInfo);
	vk->images.semaSlotIndex = (vk->images.semaSlotIndex+1)%vk->images.totalImages;
	if(globalRenderer->window->cursor!=NULL)
		SDL_SetCursor(globalRenderer->window->cursor);
#endif

	OEUpdateViewMat();
	OEClearInstanceBatchData();

	globalRenderer->frame_end = SDL_GetPerformanceCounter();
	globalRenderer->fps =
		1.0f/((globalRenderer->frame_end - globalRenderer->frame_start) / 
		(float)SDL_GetPerformanceFrequency());
	globalRenderer->frameTime = 1.0f/globalRenderer->fps;
	globalRenderer->tick+=globalRenderer->frameTime;
	if(globalRenderer->tick>=(float)MAXTICK) globalRenderer->tick = 0.0f;
	globalRenderer->frame = !globalRenderer->frame;
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
	if(globalRenderer->tick>=(float)MAXTICK) globalRenderer->tick = 0.0f;
}

void OECleanup(void) {
	OEDestroyViews(&globalRenderer->views);
	sg_destroy_view(globalRenderer->defTexture);
	sg_shutdown();
#ifndef OE_VULKAN
	if(globalRenderer->igStat) ImGui_ImplOpenGL3_Shutdown();
	SDL_GL_DeleteContext(globalRenderer->window->gl_context);
	SDL_GL_UnloadLibrary();
	SDL_DestroyWindow(globalRenderer->window->window);
	SDL_Quit();
#else
	OEVKData *vk = globalRenderer->window->VK;
	vkDeviceWaitIdle(vk->device);
	uint32_t iCount = 0;
	vkGetSwapchainImagesKHR(vk->device, vk->images.swapchain, &iCount, NULL);
	int i;
	for(i=0;i<iCount;i++) vkDestroyImageView(vk->device, vk->images.views[i], NULL);
	free(vk->images.views);
	free(vk->images.images);
	free(vk->images.renderSema);
	free(vk->images.presentSema);
	vkDestroySwapchainKHR(vk->device, vk->images.swapchain, NULL);
	vkDestroyImageView(vk->device, vk->images.depthImageView, NULL);
	vkDestroyImage(vk->device, vk->images.depthImage, NULL);
	vkFreeMemory(vk->device, vk->images.depthMem, NULL);
	SDL_Vulkan_UnloadLibrary();
	SDL_DestroyWindow(globalRenderer->window->window);
	SDL_Quit();
#endif
}
