//cbuffer CBuff
//{
//	float4 face_colors[6];
//};
//
//float4 main(uint tid : SV_PrimitiveID) : SV_TARGET
//{
//	return face_colors[tid / 2];
//
//}

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0); //register(t0) means use the first register slot for texture
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 pixelColor = objTexture.Sample(objSamplerState, input.tex);
	return float4(pixelColor, 1.0f);
}