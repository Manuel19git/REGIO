#include "common.hlsl"

// This would be per object if gTransform had World transformation matrix,
// but because all objects come in world coordinates (.obj) I don't have world matrix yet
cbuffer cbPerObject : register(b0)
{
	matrix gTransform; //matrix is 4x4
    matrix gTransformSun;
    MaterialGPU material;
    int hasTexture;
};


struct VS_INPUT
{
	float3 inPos : POSITION;
	float3 inNorm : NORMAL;
	float2 inTex : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 posOrig : POSITION; //World Space
	float3 norm : NORMAL; //World Space
	float2 tex : TEXCOORD;
    float4 shadowPosNDC : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.posOrig = input.inPos;
    output.pos = mul(float4(input.inPos, 1.0f), gTransform);
    output.norm = input.inNorm;
    output.tex = input.inTex;
    output.shadowPosNDC = mul(float4(input.inPos, 1.0f), gTransformSun);
    return output;
}