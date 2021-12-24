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
	float3 color : COLOR0;
	float3 texCoord : TEXCOORD0;
};

// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 texCoord : TEXCOORD0;
	float3 vPos : TEXCOORD1;
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

	float4 vPos = float4(input.pos, 1.0f);
	vPos = mul(vPos, model);
	vPos = mul(vPos, view);
	output.vPos = vPos.xyz;

	// Сквозная передача цвета без изменения.
	output.color = input.color;

	output.texCoord = input.texCoord;

	return output;
}
