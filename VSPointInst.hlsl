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
	float2 center : POSITION1; //texcoord space
};

// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
	float2 texCoord : TEXCOORD;

};

// Простой шейдер для обработки вершин на графическом процессоре.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	//swapChain space to screenSpace in term of VS
	float2 screenSpaceVertexPosition = 0.0f;
	screenSpaceVertexPosition.x = (input.center.x - 0.5f) * 2;
	screenSpaceVertexPosition.y = (input.center.y - 0.5f) * 2;
	
	float3 pos = float3(input.pos);
	pos.xy /= 50*scale;


	// -> to tex coord coordinates 
	pos.xy =  ((pos.xy + screenSpaceVertexPosition) + 1.0f)/2.0f; 	
	// -> shift edges
	pos.xy += -float2(xEdge, 1.0f-1.0f/scale-yEdge); 
	// -> transsform
	pos.xy *= scale;
	// -> to scren space coordinates
	pos.xy = (pos.xy - 0.5f) * 2.0f;

	output.color = input.color;
	output.pos = float4(pos, 1.0f);
	output.texCoord = input.pos.xy;

	return output;
}
