cbuffer PointParameters :register (b0)
{
	float3 color;
	float2 center;


};
// Цветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};


// Сквозная функция для (интерполированных) данных о цвете.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float xL = input.pos.x - center.x;
	float yL = input.pos.y - center.y;
	float maxRadius = 7.0f; //in pixels

	float4 resultValue = float4(color, 0.0f);

	float length = sqrt((xL * xL) + (yL * yL));
	if (length < maxRadius) resultValue.a = 1.0f - max(pow((length/ maxRadius), 8), 0); // expensive

	return float4(resultValue);
}
