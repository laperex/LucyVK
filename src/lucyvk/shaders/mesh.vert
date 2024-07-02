#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout(set = 0, binding = 1) uniform CameraBuffer {
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 color;
} camera_data;

// push 
// layout( push_constant ) uniform constants
// {
// 	vec4 data;
// 	mat4 render_matrix;
// } PushConstants;

void main()
{
	// gl_Position = PushConstants.render_matrix * vec4(vPosition, 1.0f);
	gl_Position = camera_data.projection * camera_data.view * camera_data.model * vec4(vPosition, 1.0f);
	outColor = vColor;
}
