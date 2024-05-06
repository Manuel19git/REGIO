#pragma once
#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	float3 gEyePosW;
};

cbuffer cbPerObject
{
	matrix gTransform; //matrix is 4x4
	Material gMaterial;
};

struct VS_INPUT
{
	float3 inPos : POSITION;
	float3 inNorm : NORMAL;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.inPos, 1.0f), gTransform);
	output.norm = mul(input.inNorm, (float3x3)gTransform);
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	input.norm = normalize(input.norm);
	float3 toEye = normalize(gEyePosW - input.pos.xyz);

	//This is what we are going to use to compute pixel color
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//This is the ambient,diffuse,specular values computed
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, input.norm, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	float4 litColor = ambient + diffuse + specular;

	//Common to take alpha from diffuse material -> Luna's book
	litColor.a = gMaterial.Diffuse.a;

	return litColor;
}

technique11 LighTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}