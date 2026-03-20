#include "common.hlsl"

//Nonnumeric values cannot be added to a cbuffer
TextureCube textureObject : register(t0);
SamplerState samplerState : register(s0);

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
    return textureObject.Sample(samplerState, input.posOrig);

}