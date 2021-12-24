// Буфер констант, в котором хранятся три базовых матрицы в порядке по столбцам для составления геометрии.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Данные для отдельных вершин, используемые для передачи в шейдер вершин.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 texCoord : TEXCOORD;
};

// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

// Простой шейдер для обработки вершин на графическом процессоре.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = float4(input.pos, 1.0f);
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	output.texCoord = input.texCoord;

	// Сквозная передача цвета без изменения.

	return output;
}
