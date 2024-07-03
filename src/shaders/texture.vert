#version 450

//output variable to the fragment shader
// layout (location = 2) in vec3 v_color;
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;
layout (location = 2) in vec3 v_uv;

layout (location = 0) out vec3 f_norm;
layout (location = 1) out vec3 f_uv;


layout(set = 0, binding = 1) uniform CameraBuffer {
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 color;
} ubo;


out gl_PerVertex {
    vec4 gl_Position;
};


void main()
{
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(v_pos, 1.0f);
	f_norm = v_norm;
	f_uv = v_uv;
	// outUV = uv[gl_VertexIndex % 6];
}
