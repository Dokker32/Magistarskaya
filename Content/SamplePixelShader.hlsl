// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D resultTexture : register (t0);
SamplerState mainSampler : register	(s0);

// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 resultValue = resultTexture.Sample(mainSampler, input.texCoord);

	return float4(resultValue);
}
