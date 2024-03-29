// ����� ��������, � ������� �������� ��� ������� ������� � ������� �� �������� ��� ����������� ���������.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// ������ ��� ��������� ������, ������������ ��� �������� � ������ ������.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 texCoord : TEXCOORD;
};

// �������� ������ ��������� ������, ����������� ����� ������ ��������.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

// ������� ������ ��� ��������� ������ �� ����������� ����������.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = float4(input.pos, 1.0f);
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	output.texCoord = input.texCoord;

	// �������� �������� ����� ��� ���������.

	return output;
}
