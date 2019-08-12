cbuffer CbObject : register(b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
	float4x4 worldViewProj;
};

struct VS_IN
{
	float3 pos : position;
	float4 color : color;
};

struct VS_OUT
{
	float4 pos : sv_position;
	float4 color : color;
};

VS_OUT VS(VS_IN input)
{
	VS_OUT vout;

	vout.pos = mul(float4(input.pos, 1.0f), worldViewProj);
	vout.color = input.color;

	return vout;
}

float4 PS(VS_OUT input) : sv_target
{
	return input.color;
}