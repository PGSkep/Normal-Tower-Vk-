#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	uint modelMatrixIndex;
	float red;
	float green;
	float blue;
} pushConstants;
layout(binding = 0) uniform ViewProjection
{
	mat4 view;
	mat4 projection;
} vp;
layout(binding = 1) uniform ModelMatrices
{
	mat4 matrices[1];
} modelMatrices;

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;



layout(location = 0) out vec3 Position_worldspace;
layout(location = 1) out vec2 UV;

layout(location = 4) out vec3 LightDirection_cameraspace;
layout(location = 2) out vec3 EyeDirection_cameraspace;

layout(location = 5) out vec3 LightDirection_tangentspace;
layout(location = 3) out vec3 EyeDirection_tangentspace;

layout(location = 6) out vec3 LightPosition_worldspace;

void main()
{
	mat4 modelMatrix = modelMatrices.matrices[pushConstants.modelMatrixIndex];
	mat3 MV3x3 = mat3(vp.view * modelMatrix);
	
// worldspace
	Position_worldspace = vec3(modelMatrix * vec4(vertexPosition_modelspace, 1.0));
	UV = vertexUV;
	LightPosition_worldspace = normalize(vec3(pushConstants.red, pushConstants.green, pushConstants.blue)) * 10;

// cameraspace
	vec3 vertexNormal_cameraspace    = MV3x3 * vertexNormal_modelspace;
	vec3 vertexTangent_cameraspace   = MV3x3 * vertexTangent_modelspace;
	vec3 vertexBitangent_cameraspace = MV3x3 * vertexBitangent_modelspace;

	vec3 LightPosition_cameraspace  = ( vp.view * vec4(LightPosition_worldspace,1)).xyz;
	vec3 vertexPosition_cameraspace = ( vp.view * modelMatrix * vec4(vertexPosition_modelspace,1)).xyz;

	EyeDirection_cameraspace   = vec3(0,0,0) - vertexPosition_cameraspace;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

// tangentspace
	mat3 TBN = transpose(mat3(
		vertexTangent_cameraspace,
		vertexBitangent_cameraspace,
		vertexNormal_cameraspace));

	LightDirection_tangentspace = TBN * LightDirection_cameraspace;
	EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;

	gl_Position = vp.projection * vp.view * modelMatrix * vec4(vertexPosition_modelspace, 1.0);
}