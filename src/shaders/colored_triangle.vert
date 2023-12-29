#version 450

//output variable to the fragment shader
layout (location = 0) out vec3 outColor;

layout( push_constant ) uniform constants
{
	vec4 offset;
	mat4 render_matrix;
} PushConstants;

void main()
{
	//const array of positions for the triangle
	const vec3 positions[6] = vec3[6](
		vec3(1.0f, 1.0f,  0.0f),
		vec3(0.0f, 0.0f,  0.0f),
		vec3(1.0f, 0.0f,  0.0f),

		vec3(1.0f, 1.0f,  0.0f),
		vec3(0.0f, 1.0f,  0.0f),
		vec3(0.0f, 0.0f,  0.0f)
	);

	//const array of colors for the triangle
	const vec3 colors[6] = vec3[6](
		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(1.0f, 0.0f, 0.0f), //red
		
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(0.0f, 1.0f, 0.0f) //green
	);

	gl_Position = PushConstants.render_matrix * vec4(positions[gl_VertexIndex % 6] + PushConstants.offset.xyz, 1.0f);
	outColor = colors[gl_VertexIndex % 6];
}
