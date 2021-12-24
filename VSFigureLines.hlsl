cbuffer ScaleConstantBuffer : register(b0)
{
	float scale;
	float yEdge; //texcoord space
	float xEdge; //texcoord space
	float padding;
};
// Данные для отдельных вершин, используемые для передачи в шейдер вершин.
struct VertexShaderInput
{
	float3 pos : POSITION0;
	float3 color : COLOR;
};

// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;

};

// Простой шейдер для обработки вершин на графическом процессоре.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	output.color = input.color;
	float2 pos = input.pos.xy;
	// -> to tex coord coordinates 
	pos.xy = ((pos.xy) + 1.0f) / 2.0f;
	// -> shift edges
	pos.xy += -float2(xEdge, 1.0f - 1.0f / scale - yEdge);
	// -> transsform
	pos.xy *= scale;
	// -> to scren space coordinates
	pos.xy = (pos.xy - 0.5f) * 2.0f;
	output.pos = float4(pos, 0.0f,1.0f);

	return output;
}
