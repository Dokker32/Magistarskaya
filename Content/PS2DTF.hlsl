cbuffer ScaleConstantBuffer : register(b0)
{
	float scale;
	float yEdge;
	float xEdge;
	float exposure;
};


// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};


Texture2D<uint> histTexture : register (t0);
SamplerState mainSampler : register	(s0);

// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{

	float histSize = 511.0f;
	float2 texCoord = input.texCoord/scale;
	texCoord.x += xEdge;
	texCoord.y += yEdge;
	float histValue = histTexture.Load(int3(texCoord* histSize, 0));
	histValue = 1.0 - exp(-histValue * exposure);
	return float4( histValue, 0.0f, 0.0f, 1.0f);
}
