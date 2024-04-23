cbuffer CBuff
{
	matrix transform; //matrix is 4x4
};

struct VS_INPUT
{
	float3 inPos : POSITION;
	float2 inTex : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 outPos : SV_POSITION;
	float2 outTex : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.outPos = mul(float4(input.inPos, 1.0f), transform);
	output.outTex = input.inTex;
	return output;
}