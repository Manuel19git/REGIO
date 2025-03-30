#include "common.hlsl"

//Nonnumeric values cannot be added to a cbuffer
Texture2D textureObject : register(t0);

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
    float depth = textureObject.Sample(samTriLinearSam, input.tex).r;
    return float4(depth, depth, depth, 1.0f);
};