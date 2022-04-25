
struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

float4 main(PS_INPUT inp) : SV_TARGET
{
	return inp.color;
}