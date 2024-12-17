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
    matrix gTransformSkybox;
    matrix gTransformSun;
	Material gMaterial;
};

//Nonnumeric values cannot be added to a cbuffer
Texture2D textureObject;
Texture2D shadowMap;
TextureCube textureCubemap;

// Array of point lights
PointLight gPointLights[6];

SamplerState objSamplerState
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samTriLinearSam
{
    Filter = MIN_MAG_MIP_LINEAR;
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

DepthStencilState lessEqualDSS
{
    DepthFunc = LESS_EQUAL;
};

RasterizerState Depth
{
    DepthBias = 10000;
    DepthBiasClamp = 0.0f;
    SlopeScaledDepthBias = 1.0f;
};

float4 PS_multilight(VS_OUTPUT input, uniform bool useTexture) : SV_TARGET
{
	input.norm = normalize(input.norm);
	float3 toEye = normalize(gEyePosW - input.posOrig.xyz);

	float4 texColor = float4(1, 1, 1, 1);
	if (useTexture)
	{
        texColor = textureObject.Sample(samTriLinearSam, input.tex);
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
	return litColor;
}


struct VS_TEXTURE_INPUT
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD;
};
struct VS_TEXTURE_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};


VS_TEXTURE_OUTPUT VS_Texture(VS_TEXTURE_INPUT input)
{
    VS_TEXTURE_OUTPUT output;
    //output.pos = input.pos;
    output.pos = mul(float4(input.pos, 1.0f), gTransformSkybox);
    output.tex = input.tex;
	
    return output;
}

float4 PS_Texture(VS_TEXTURE_OUTPUT input) : SV_TARGET
{
    float depth = shadowMap.Sample(objSamplerState, input.tex).r;
    return float4(depth, depth, depth, 1.0f);
}

VS_OUTPUT VS_Skybox(VS_INPUT input)
{
    VS_OUTPUT output;
    output.posOrig = input.inPos;
    output.pos = mul(float4(input.inPos, 1.0f), gTransformSkybox);
    output.norm = input.inNorm;
	output.tex = input.inTex;
	return output;
}


float4 PS_Skybox(VS_OUTPUT input) : SV_TARGET
{
    return textureCubemap.Sample(samTriLinearSam, input.posOrig);
}

VS_OUTPUT VS_Sun(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(float4(input.inPos, 1.0f), gTransform);
    //output.pos = mul(float4(input.inPos, 1.0f), gTransformSun);
	output.tex = input.inTex;
	return output;
}

technique11 LighTech
{
	// Try creating a previous pass for getting the shadow map
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Sun()));
        SetPixelShader(NULL);
        SetRasterizerState(Depth);
    }
	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_multilight(false)));
    }
}

technique11 LighTechTex
{
  //  pass P0
  //  {
		//SetVertexShader(CompileShader(vs_5_0, VS_Sun()));
  //      SetPixelShader(NULL);
		
  //      SetRasterizerState(Depth);
  //  }

	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_multilight(true)));
	}
}

technique11 DebugTexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Texture()));
        SetPixelShader(CompileShader(ps_5_0, PS_Texture()));
    }
}

technique11 Sky
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skybox()));
        SetPixelShader(CompileShader(ps_5_0, PS_Skybox()));
        SetDepthStencilState(lessEqualDSS, 0); // I need this to work with shadow maps

    }
}