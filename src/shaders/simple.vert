#version 450

// layout (location=0) in vec2 pos;
// layout (location=1) in vec3 color;

layout (location=0) out vec3 f_color;

vec2 c_pos[3] = vec2[] (
	vec2(0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

void main() {
	// gl_Position = vec4(pos, 0.0, 1.0);
	// f_color = color;
	gl_Position = vec4(c_pos[gl_VertexIndex], 0.0, 1.0);
	f_color = vec3(1, 1, 1);
}
