// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};


// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 resultValue = float4(input.color, 1.0f);
	return float4(resultValue);
}
