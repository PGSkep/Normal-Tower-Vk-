#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D texSampler;

layout(location = 0) in vec3 Position_worldspace;
layout(location = 1) in vec2 UV;

layout(location = 2) in vec3 EyeDirection_cameraspace;
layout(location = 3) in vec3 EyeDirection_tangentspace;

layout(location = 4) in vec3 LightDirection_cameraspace;
layout(location = 5) in vec3 LightDirection_tangentspace;

layout(location = 6) in vec3 LightPosition_worldspace;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3  LightColor = vec3(1,1,1);
	float LightPower = 40.0;

	vec3 MaterialAmbientColor  = vec3(0.0, 0.0, 0.1);
	vec3 MaterialDiffuseColor  = vec3(0.5, 0.5, 0.5);
	vec3 MaterialSpecularColor = vec3(1.0, 1.0, 1.0);

	vec3 TextureNormal_tangentspace = normalize(texture(texSampler, UV).rgb * 2.0 - 1.0);

	float distance = sqrt(
		(LightPosition_worldspace.x - Position_worldspace.x) *
		(LightPosition_worldspace.x - Position_worldspace.x) +
		(LightPosition_worldspace.y - Position_worldspace.y) *
		(LightPosition_worldspace.y - Position_worldspace.y) +
		(LightPosition_worldspace.z - Position_worldspace.z) *
		(LightPosition_worldspace.z - Position_worldspace.z));

	vec3 n = TextureNormal_tangentspace;
	vec3 l = normalize(LightDirection_tangentspace);
	float cosTheta = clamp( dot( n,l ), 0,1 );
	vec3 E = normalize(EyeDirection_tangentspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	outColor = vec4(
		MaterialAmbientColor +
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,100) / (distance*distance)
		, 1);

	//outColor = vec4(LightPosition_worldspace, 1.0);
}