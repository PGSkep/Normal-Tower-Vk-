#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	uint modelMatrixIndex;
	float red;
	float green;
	float blue;

} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(binding = 0) uniform ViewProjection
{
	mat4 view;
	mat4 projection;
} vp;
layout(binding = 1) uniform ModelMatrices
{
	mat4 matrices[1];
} modelMatrices;

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;

layout(location = 5) out vec3 outFragPos;
layout(location = 6) out vec3 outViewPos;
layout(location = 7) out vec3 outLightPos;
layout(location = 8) out vec3 outViewForward;

void main()
{
	gl_Position = vp.projection * vp.view * modelMatrices.matrices[pushConstants.modelMatrixIndex] * vec4(inPosition, 1.0);
	outUv = inUv;
	outColor = vec3(pushConstants.red, pushConstants.green, pushConstants.blue);
	outNormal = inNormal;
	outTangent = inTangent;
	outBitangent = inBitangent;

	outFragPos = vec3(modelMatrices.matrices[pushConstants.modelMatrixIndex] * vec4(inPosition, 1.0f));
	outViewPos = vec3(vp.view[3][0], vp.view[3][1], vp.view[3][2]);
	outLightPos = vec3(vec4(pushConstants.red, pushConstants.green, pushConstants.blue, 1.0f));
	
	outViewForward = vec3(vp.view[0][2], vp.view[1][2], vp.view[2][2]);
}