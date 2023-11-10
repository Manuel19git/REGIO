struct VSOut
{
	float3 color: Color;
	float4 pos: SV_POSITION; //System Value Position
};

cbuffer CBuff
{
	row_major matrix transform; //matrix is 4x4
};

VSOut main( float2 pos : POSITION, float3 color : COLOR)
{
	VSOut vsout;
	vsout.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform);
	vsout.color = color;
	return vsout;
}