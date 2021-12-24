cbuffer CursorPosition : register (b0) {
	float2 cursorPosition;
	float2 padding;
}
// ������ ��� ��������� ������, ������������ ��� �������� � ������ ������.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD;
};

// �������� ������ ��������� ������, ����������� ����� ������ ��������.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

// ������� ������ ��� ��������� ������ �� ����������� ����������.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	//swapChain space to screenSpace in term of VS and PS
	float2 screenSpaceCursorPosition;
	screenSpaceCursorPosition.x = (cursorPosition.x - 0.5f) * 2;
	screenSpaceCursorPosition.y = (cursorPosition.y + 0.5f) * 2;
	
	float4 pos = float4(input.pos, 1.0f);
	pos.xy /= 50;

	pos.xy = pos.xy + screenSpaceCursorPosition;
	output.pos = pos;
	output.texCoord = input.texCoord;


	return output;
}
