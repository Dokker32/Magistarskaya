// �������� ������ ��������� ������, ����������� ����� ������ ��������.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};


// �������� ������� ��� (�����������������) ������ � �����.
float4 main(PixelShaderInput input) : SV_TARGET
{	

	return float4(input.texCoord, 1.0f);
}
