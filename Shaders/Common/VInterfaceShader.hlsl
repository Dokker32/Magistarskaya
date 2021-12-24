// ������ ��� ��������� ������, ������������ ��� �������� � ������ ������.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD0;
};

// �������� ������ ��������� ������, ����������� ����� ������ ��������.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

// ������� ������ ��� ��������� ������ �� ����������� ����������.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;


	output.pos = float4(input.pos.xy, 0.0f, 1.0f);
	output.texCoord = input.texCoord;

	return output;
}
