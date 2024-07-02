#version 450

layout (location = 0) in vec3 v_pos;

layout(set = 0, binding = 1) uniform CameraBuffer {
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 color;
} camera_data;

void main() {
	gl_Position = camera_data.projection * camera_data.view * camera_data.model * vec4(v_pos, 1.0f);
}
