#version 450

//shader input
layout (location = 0) in vec3 f_norm;
layout (location = 1) in vec3 f_uv;

layout(set = 0, binding = 2) uniform sampler2D _sampler;

//output write
layout (location = 0) out vec4 o_color;


void main()
{
	vec3 color = texture(_sampler, f_uv.xy).rgb;
	o_color = vec4(color, 1.0f);
}
