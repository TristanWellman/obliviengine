@ctype mat4 mat4x4
@ctype vec3 vec3

@vs vs
layout(binding=0) uniform vs_params {
    mat4 model;             
    mat4 view;              
    mat4 proj;              
    mat4 mvp;               
    vec3 position;          
    vec3 target;            
    vec3 up;                
    vec3 front;             
    vec3 right; 
	float aspect;
};

in vec4 vert_position; 
out vec3 ray_origin;
out vec3 ray_dir;

void main() {
    gl_Position = vert_position;
    
    vec2 ndc = vert_position.xy;
    
    float fov = radians(60.0);
    float scale = tan(fov * 0.5);
    
    vec3 direction = normalize(front + ndc.x * scale * right * aspect + ndc.y * scale * up);
    
    ray_origin = position;
    ray_dir = direction;
}
@end

@fs fs

precision mediump float;

#pragma

#define MAX_VOXELS 300
#define MAX_LIGHTS 300
#define MAX_BOUNCES 1
#define VOXSIZE 1
#define MAXDIST 100.0

layout(binding=1) uniform light_data {
    vec4 light_positions[MAX_LIGHTS]; // xyz: position, w: intensity
    vec4 light_colors[MAX_LIGHTS];    // rgb: color, a: unused
    int num_lights;                    
};

layout(binding=2) uniform voxel_data {
    vec4 voxel_info[MAX_VOXELS]; // xyz: position, w: size
    int num_voxels;
};

layout(binding=3) uniform texture2D v_texture;
layout(binding=3) uniform sampler smp;

in vec3 ray_origin;
in vec3 ray_dir;

out vec4 frag_color;

struct HitInfo {
    float t;
    vec3 position;
    vec3 normal;
    int voxel_id;
};

bool is_visible(vec3 pos, vec3 norm) {
	vec3 neighbor_pos = pos + norm * VOXSIZE;
	int i;
    for(i = 0; i < num_voxels; i++) {
        vec3 neighbor_voxel = voxel_info[i].xyz;
        if (all(equal(neighbor_voxel, neighbor_pos))) return false;
    }

    return true;
}

bool intersect_voxel(vec3 origin, vec3 dir, vec4 voxel, out HitInfo hit) {
    vec3 pos = voxel.xyz;
    float size = voxel.w*0.5;
    
    vec3 min_bound = pos-vec3(size);
    vec3 max_bound = pos+vec3(size);
    
    float tmin = (min_bound.x-origin.x)/dir.x;
    float tmax = (max_bound.x-origin.x)/dir.x;
    if(tmin>tmax) { float temp = tmin; tmin = tmax; tmax = temp; }
    
    float tymin = (min_bound.y-origin.y)/dir.y;
    float tymax = (max_bound.y-origin.y)/dir.y;
    if(tymin>tymax) { float temp = tymin; tymin = tymax; tymax = temp; }
    
    if ((tmin > tymax) || (tymin > tmax)) return false;
    
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;
    
    float tzmin = (min_bound.z - origin.z) / dir.z;
    float tzmax = (max_bound.z - origin.z) / dir.z;
    if(tzmin > tzmax) { float temp = tzmin; tzmin = tzmax; tzmax = temp; }
    
    if((tmin > tzmax) || (tzmin > tmax)) return false;
    
    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;
    
    if (tmin < 0.0) {
        if (tmax < 0.0) return false;
        tmin = tmax;
    }
    
    hit.t = tmin;
    hit.position = origin + tmin * dir;
    
    // Calculate normal at the hit point
    vec3 p = hit.position;
    float epsilon = 1e-4;
    if(abs(p.x - min_bound.x) < epsilon) { hit.normal = vec3(-1, 0, 0); }
    else if(abs(p.x - max_bound.x) < epsilon) { hit.normal = vec3(1, 0, 0); }
    else if(abs(p.y - min_bound.y) < epsilon) { hit.normal = vec3(0, -1, 0); }
    else if(abs(p.y - max_bound.y) < epsilon) { hit.normal = vec3(0, 1, 0); }
    else if(abs(p.z - min_bound.z) < epsilon) { hit.normal = vec3(0, 0, -1); }
    else { hit.normal = vec3(0, 0, 1); }
   
	if (!is_visible(voxel.xyz, hit.normal)) return false;

    return true;
}

HitInfo closest_hit(vec3 origin, vec3 dir) {
    HitInfo closest;
    closest.t = 1e20;
    closest.voxel_id = -1;
    int i;
    for(i=0;i<num_voxels;i++) {
        HitInfo hit;
        if(intersect_voxel(origin, dir, voxel_info[i], hit)) {
            if(hit.t < closest.t) {
                closest = hit;
                closest.voxel_id = i;
            }
        }
    }
    return closest;
}

bool any_intersection(vec3 origin, vec3 dir, float max_t) {
	int i;
    for(i=0;i<num_voxels;i++) {
		if(voxel_info[i].x-origin.x>2) break;
        HitInfo hit;
        if(intersect_voxel(origin, dir, voxel_info[i], hit)) {
            if(hit.t < max_t && hit.t > 1e-4) return true;
        }
    }
    return false;
}

vec3 compute_lighting(HitInfo hit, out bool is_shadow) {
	vec3 pos = hit.position;
	vec3 normal = hit.normal;
    vec3 color = vec3(0.0);
	int i;
    for(i=0;i<num_lights;i++) {
        vec3 light_pos = light_positions[i].xyz;
        float intensity = light_positions[i].w;
        vec3 light_color = light_colors[i].rgb;
        
        vec3 to_light = light_pos - pos;
        float distance = length(to_light);
       
        // Early exit for lights too far away
        if (length(light_pos - pos) > MAXDIST) continue;

	   	vec3 dir_to_light = normalize(to_light);
        
        vec3 shadow_orig = pos + normal * 1e-4;
        
        bool in_shadow = any_intersection(shadow_orig, dir_to_light, distance);
        
        if(!in_shadow) {
            float lambert = max(dot(normal, dir_to_light), 0.0);
            color += lambert * intensity * light_color;
			is_shadow = false;
        } else {
			is_shadow = true;
		};
    }
    return color;
}

void main() {
    vec3 final_color = vec3(0.0);
    vec3 throughput = vec3(1.0);

    vec3 origin = ray_origin;
    vec3 dir = ray_dir;

	bool is_shadow = false;

    for (int bounce = 0; bounce < MAX_BOUNCES; bounce++) {
        HitInfo hit = closest_hit(origin, dir);
        if (hit.voxel_id == -1) break;

        vec4 voxel = voxel_info[hit.voxel_id];
        vec3 min_bound = voxel.xyz - vec3(voxel.w * 0.5);
        vec3 max_bound = voxel.xyz + vec3(voxel.w * 0.5);

        vec2 tex_coords;
        if (abs(hit.normal.x) > 0.9) {
            tex_coords = vec2(hit.position.z, hit.position.y);
            tex_coords = (tex_coords - min_bound.zy) / (max_bound.zy - min_bound.zy);
        } else if (abs(hit.normal.y) > 0.9) {
            tex_coords = vec2(hit.position.x, hit.position.z);
            tex_coords = (tex_coords - min_bound.xz) / (max_bound.xz - min_bound.xz);
        } else {
            tex_coords = vec2(hit.position.x, hit.position.y);
            tex_coords = (tex_coords - min_bound.xy) / (max_bound.xy - min_bound.xy);
        }

        vec3 tex_color = texture(sampler2D(v_texture, smp), tex_coords).rgb;

        vec3 light_contrib = compute_lighting(hit, is_shadow);

        final_color += throughput * light_contrib * tex_color;

        origin = hit.position + hit.normal * 1e-4;
        dir = reflect(dir, hit.normal);

        throughput *= 0.1;
        if(length(throughput) < 0.01 && bounce > 1) break;
    }

    //if (length(final_color) < 0.01 && !is_shadow) {
    //    frag_color = vec4(0.0, 0.0, 0.0, 0.0); // transparent
    //} else {
        frag_color = vec4(final_color, 1.0); 
    //}
}


@end

@program light vs fs
