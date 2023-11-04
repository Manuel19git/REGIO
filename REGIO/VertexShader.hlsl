struct VSOut
{
	float3 color: Color;
	float4 pos: SV_POSITION; //System Value Position
};

VSOut main( float2 pos : POSITION, float3 color : COLOR)
{
	VSOut vsout;
	vsout.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
	vsout.color = color;
	return vsout;
}