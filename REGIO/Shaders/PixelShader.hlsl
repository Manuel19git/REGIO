#include "common.hlsl"

cbuffer cbPerObject : register(b0)
{
	matrix gTransform; //matrix is 4x4
    matrix gTransformSun;
	Material gMaterial;
    int hasTexture;
};

cbuffer cbPerFrame : register(b1)
{
	DirectionalLight gDirLight;
    PointLight gPointLights[6];
    SpotLight gSpotLight;
	float3 gEyePosW;
    int sunActive;
};

//Nonnumeric values cannot be added to a cbuffer
Texture2D shadowMap : register(t0);
Texture2D textureObject : register(t1);
SamplerComparisonState samShadow : register(s0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 posOrig : POSITION; //World Space
	float3 norm : NORMAL; //World Space
	float2 tex : TEXCOORD;
    float4 shadowPosNDC : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    input.norm = normalize(input.norm);
	float3 toEye = normalize(gEyePosW - input.posOrig.xyz);

    float4 texColor = float4(1, 1, 1, 1);
	if (hasTexture)
	{
        texColor = textureObject.Sample(samTriLinearSam, input.tex);
    }
	
	////////////
	//Lighting//
	////////////
    float shadow = 1.0f;
    if (sunActive)
    {
        shadow = CalcShadowFactor(samShadow, shadowMap, input.shadowPosNDC);
    }
    //texColor = shadowMap.Sample(samTriLinearSam, input.tex);

	//This is what we are going to use to compute pixel color
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//This is the ambient,diffuse,specular values computed
	float4 A, D, S;

    ComputeDirectionalLight(gMaterial, gDirLight, input.norm, toEye, A, D, S);
    ambient += A;
    diffuse += shadow * D;
    specular += shadow * S;
    for (int i = 0; i < 6; ++i)
    {
        ComputePointLight(gMaterial, gPointLights[i], input.posOrig, input.norm, toEye, A, D, S);
        ambient += A;
        diffuse += D;
        specular += S;
    }
    ComputeSpotLight(gMaterial, gSpotLight, input.posOrig, input.norm, toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
	
	//It seems like texColor only affects ambient and diffuse
    float4 litColor = texColor * (ambient + diffuse) + specular;

	//Common to take alpha from diffuse material -> Luna's book
	litColor.a = gMaterial.Diffuse.a;
    //litColor = float4(shadow, shadow, shadow, 1.0f);
    return litColor;
}