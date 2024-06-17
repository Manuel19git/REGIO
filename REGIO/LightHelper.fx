#pragma once

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
};

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Direction;
	float Pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
    float3 Direction;
    float spot;
	
    float3 Att;
    float Pad;
};

void ComputeDirectionalLight(Material mat, DirectionalLight light,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//light vector aims opposite the direction the light rays travel -> Luna's book
	float3 lightVec = -light.Direction;

	ambient = mat.Ambient * light.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specularFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
		specular = specularFactor * mat.Specular * light.Specular;
	}
}

void ComputeSpotLight(Material mat, SpotLight light,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//light vector aims opposite the direction the light rays travel -> Luna's book
    float3 lightVec = light.Position - pos;
	
	//Don't opperate light too far from object
    float distance = length(lightVec);
	if (distance > light.Range)
        return;
	
	//Normalize light vector
    lightVec /= distance;

    ambient = mat.Ambient * light.Ambient;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectedVec = reflect(-lightVec, normal);
        float specularFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
        specular = specularFactor * mat.Specular * light.Specular;
    }
	
	//Scale by spotlight factor and attenuate
    float spot = pow(max(dot(-lightVec, light.Direction), 0.0f), light.spot);
	
	//Scale by spotlight factor and attenuate
    float att = spot / dot(light.Att, float3(1.0f, distance, distance * distance));
	
    ambient *= spot;
    diffuse *= att;
    specular *= att;
}