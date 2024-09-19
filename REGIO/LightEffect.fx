#pragma once
#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
	float3 gEyePosW;
};

cbuffer cbPerObject
{
	matrix gTransform; //matrix is 4x4
	Material gMaterial;
};

//Nonnumeric values cannot be added to a cbuffer
Texture2D textureObject;

SamplerState objSamplerState
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
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
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
    output.posOrig = input.inPos;
	output.pos = mul(float4(input.inPos, 1.0f), gTransform);
    output.norm = input.inNorm;
    //output.norm = mul(input.inNorm, (float3x3)gTransform);
	output.tex = input.inTex;
	return output;
}

float4 PS(VS_OUTPUT input, uniform bool useTexture) : SV_TARGET
{
	input.norm = normalize(input.norm);
	float3 toEye = normalize(gEyePosW - input.posOrig.xyz);

	float4 texColor = float4(1, 1, 1, 1);
	if (useTexture)
	{
		texColor = textureObject.Sample(objSamplerState, input.tex);
	}

	////////////
	//Lighting//
	////////////

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
	ComputePointLight(gMaterial, gPointLight, input.posOrig, input.norm, toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    ComputeSpotLight(gMaterial, gSpotLight, input.posOrig, input.norm, toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
	
	//It seems like texColor only affects ambient and diffuse
	float4 litColor = texColor * (ambient + diffuse) + specular;

	//Common to take alpha from diffuse material -> Luna's book
	litColor.a = gMaterial.Diffuse.a;
    float3 lightVec = gPointLight.Position - input.posOrig;
    float distance = length(lightVec);
    //litColor = float4(distance / gPointLight.Range , distance /gPointLight.Range, distance / gPointLight.Range, 1.0f);
    //litColor = ambient;
	return litColor;
}

struct VS_SIMPLE_INPUT
{
	float4 inPos : POSITION;
	float4 inColor : COLOR;
};

struct PS_SIMPLE_INPUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

PS_SIMPLE_INPUT VS_Simple(VS_SIMPLE_INPUT input)
{
    PS_SIMPLE_INPUT output;
    //output.pos = input.inPos;
	output.pos = mul(input.inPos, gTransform);
    output.color = input.inColor;
	
    return output;
}

float4 PS_Simple(PS_SIMPLE_INPUT input) : SV_TARGET
{
    return input.color;
}

technique11 LighTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS(false)));
	}
}

technique11 LighTechTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS(true)));
	}
}

technique11 Simple
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Simple()));
        SetPixelShader(CompileShader(ps_5_0, PS_Simple()));
    }
}