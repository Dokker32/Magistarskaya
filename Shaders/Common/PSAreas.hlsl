cbuffer ScaleConstantBuffer : register(b0)
{
	float scale;
	float yEdge;
	float xEdge;
	float tonalCompression;
};


// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};


Texture2D<float4> histTexture : register (t0);
SamplerState mainSampler : register	(s0);

// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float histSize = 511.0f;
	float2 texCoord = input.texCoord/scale;
	texCoord.x += xEdge;
	texCoord.y += yEdge;
	float4 AreaValue = histTexture.SampleLevel(mainSampler, texCoord, 0);

	return AreaValue;
}
