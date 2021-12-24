// Данные для отдельных вершин, используемые для передачи в шейдер вершин.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD0;
};

// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

// Простой шейдер для обработки вершин на графическом процессоре.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;


	output.pos = float4(input.pos.xy, 0.0f, 1.0f);
	output.texCoord = input.texCoord;

	return output;
}
