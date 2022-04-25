
struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

cbuffer cb_object: register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

VS_OUTPUT main(VS_INPUT inp)
{
	VS_OUTPUT o = (VS_OUTPUT)0;
	o.pos = float4(inp.pos, 1);
	o.pos = mul(o.pos, model);
	o.pos = mul(o.pos, view);
	o.pos = mul(o.pos, projection);
	o.color = inp.color;
	o.normal = inp.normal;
	return o;
}