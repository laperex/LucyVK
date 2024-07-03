#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout(set = 0, binding = 2) uniform sampler2D samplerCOlor;

//output write
layout (location = 0) out vec4 outFragColor;


void main()
{
	//return color
	vec3 color = texture(samplerCOlor, inUV).rgb;
	// vec3 color = inColor;
	outFragColor = vec4(color, 1.0f);
}
