// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
	float2 texCoord : TEXCOORD;
};


// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float xL = input.texCoord.x;
	float yL = input.texCoord.y;
	float maxRadius = 0.5f; //% of polygons

	float4 resultValue = float4(input.color, 0.0f);

	float length = sqrt((xL * xL) + (yL * yL));
	if (length < maxRadius) resultValue.a = 1.0f - max(pow((length/0.5f), 8), 0); // expensive

	//resultValue = float4(input.texCoord, 0.0f, 1.0f);
	return float4(resultValue);
}
