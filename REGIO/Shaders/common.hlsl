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
	float Intensity;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
    float3 Att;
    float Intensity;
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
    float Intensity;
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

    ambient = mat.Ambient * light.Ambient * light.Intensity;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectedVec = reflect(-lightVec, normal);
        float specularFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * light.Diffuse * light.Intensity;
        specular = specularFactor * mat.Specular * light.Specular * light.Intensity;
    }
}

 void ComputePointLight(Material mat, PointLight light,
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

    ambient = mat.Ambient * light.Ambient * light.Intensity;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectedVec = reflect(-lightVec, normal);
        float specularFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * light.Diffuse * light.Intensity;
        specular = specularFactor * mat.Specular * light.Specular * light.Intensity;
    }

	//Scale by spotlight factor and attenuate
    float att = 1.0f / dot(light.Att, float3(1.0f, distance, distance * distance));

    diffuse *= att;
    specular *= att;
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
	
    ambient = mat.Ambient * light.Ambient * light.Intensity;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectedVec = reflect(-lightVec, normal);
        float specularFactor = pow(max(dot(reflectedVec, toEye), 0.0f), mat.Specular.w);

        diffuse = diffuseFactor * mat.Diffuse * light.Diffuse * light.Intensity;
        specular = specularFactor * mat.Specular * light.Specular * light.Intensity;
    }
	//Scale by spotlight factor and attenuate
    float spot = pow(max(dot(-lightVec, light.Direction), 0.0f), light.spot);
	
	//Scale by spotlight factor and attenuate
    float att = spot / dot(light.Att, float3(1.0f, distance, distance * distance));
	
    ambient *= spot;
    diffuse *= att;
    specular *= att;
}

// I shouldn't put SamplerState configurations here, it should be done in the CPU side
SamplerState samTriLinearSam
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

float CalcShadowFactor(SamplerComparisonState sampShadow, Texture2D shadowMap, float4 shadowPosNDC)
{
	// Projection
    shadowPosNDC.xyz /= shadowPosNDC.w;

    // NDC Space
    float2 shadowTexCoords;
    shadowTexCoords.x = shadowPosNDC.x * 0.5f + 0.5f;
    shadowTexCoords.y = -shadowPosNDC.y * 0.5f + 0.5f;

    float depth = shadowPosNDC.z;

    // PFC Filtering
    float percentLit = 0.0f;
	float2 texelSize = 1.0f / float2(2048, 2048);
	for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float2 offset = float2(x, y) * texelSize;
            percentLit += shadowMap.SampleCmpLevelZero(sampShadow, shadowTexCoords + offset, depth);
        }
    }
	// Average samples
    percentLit /= 9;
    //float textureDepth = shadowMap.Sample(samTriLinearSam, shadowTexCoords).r;
    return percentLit;
}