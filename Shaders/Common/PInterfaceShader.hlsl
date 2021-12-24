

// ÷ветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

SamplerState mainSampler : register	(s0);
Texture2D interfaceTex : register (t3);


float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 color = interfaceTex.Sample(mainSampler, input.texCoord);

	return float4(color, 1.0f);
}
