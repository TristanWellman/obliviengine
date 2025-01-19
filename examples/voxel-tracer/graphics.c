#include "graphics.h"

GraphicsData gfx;

void initScreenQ() {
	gfx.lightingShader = sg_make_shader(light_shader_desc(sg_query_backend()));
	gfx.windowPipe = sg_make_pipeline(&(sg_pipeline_desc){
	    .shader = gfx.lightingShader,
	    .layout = {
	        .attrs = {
	            [0] = { .format = SG_VERTEXFORMAT_FLOAT3 },
	        },
	    },
	   	.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
		.index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
           	.compare = SG_COMPAREFUNC_LESS_EQUAL,
           	.write_enabled = true,
        },
		.colors[0] = {
			.blend = {
        		.enabled = true,
        		.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        		.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        		.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
        		.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    		}	
		},
		.label = "screenqpipe"
	});
	float verts[] = {
    	-1.0f, -1.0f, 0.0f,  // Bottom-left
    	1.0f, -1.0f, 0.0f,  // Bottom-right
    	-1.0f,  1.0f, 0.0f,  // Top-left
		1.0f,  1.0f, 0.0f   // Top-right
	};
	uint16_t indices[] = {
		0,2,1,
		2,3,1
	};
	gfx.screenQ.vbuf = sg_make_buffer(&(sg_buffer_desc){
    	.data = SG_RANGE(verts),
    	.label = "fullscreen_quad_vertices",
	});
	gfx.screenQ.ibuf = sg_make_buffer(&(sg_buffer_desc){
				.type = SG_BUFFERTYPE_INDEXBUFFER,
				.data = SG_RANGE(indices),
				.label = "cube_indices"
			});
	gfx.screenQ.defShader = gfx.lightingShader;
	gfx.screenQ.pipe = gfx.windowPipe;
	gfx.screenQ.numIndices = ARRLEN(indices);
	gfx.screenQ.name = "screenQ";
	createObject(gfx.screenQ);
}

void updateVSPARAMS() {
/* renderer.h Camera
 *	mat4x4 model;
	mat4x4 view, proj;
	mat4x4 iso;
	mat4x4 mvp;
	vec3 position, target, up;
	vec3 front, right; 
 * */

	Camera *cam = getCamera();
	mat4x4_dup(gfx.vs_params.model, cam->model);
	mat4x4_dup(gfx.vs_params.view, cam->view);
	mat4x4_dup(gfx.vs_params.proj, cam->proj);
	mat4x4_dup(gfx.vs_params.mvp, cam->mvp);
	vec3_dup(gfx.vs_params.position, cam->position);
	vec3_dup(gfx.vs_params.target, cam->target);
	vec3_dup(gfx.vs_params.front, cam->front);
	vec3_dup(gfx.vs_params.up, cam->up);
	vec3_dup(gfx.vs_params.right, cam->right);
	gfx.vs_params.aspect = cam->aspect;
}

void gfxSetShaderParams(voxel_data_t voxel_data,
						light_data_t light_data,
						sg_image texture) {
	memcpy(&gfx.voxel_data, &voxel_data, sizeof(voxel_data));
	memcpy(&gfx.light_data, &light_data, sizeof(light_data));
	memcpy(&gfx.curTex, &texture, sizeof(texture));
}

void gfxApplyUniforms() {
	updateVSPARAMS();
	sg_apply_uniforms(UB_vs_params, &SG_RANGE(gfx.vs_params));
	sg_apply_uniforms(UB_light_data, &SG_RANGE(gfx.light_data));
	sg_apply_uniforms(UB_voxel_data, &SG_RANGE(gfx.voxel_data));
}

sg_shader *getLightShader() {
	return &gfx.lightingShader;
}

sg_pipeline *getWindowPipe() {
	return &gfx.windowPipe;
}

Object *getScreenQ() {
	return &gfx.screenQ;
}
