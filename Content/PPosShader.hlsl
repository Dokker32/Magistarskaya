// ÷ветовые данные отдельных вершин, пропущенные через шейдер пикселей.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 texCoord : TEXCOORD0;
	float3 vPos : TEXCOORD1;
};

Texture3D brainTexture : register (t0);
SamplerState mainSampler : register	(s0);

//https://www.cg.informatik.uni-siegen.de/data/Tutorials/EG2006/RTVG04_GPU_Raycasting.pdf
float4 main(PixelShaderInput input) : SV_TARGET
{	
	
	//float3 color = brainTexture.Sample(mainSampler, input.texCoord);
	return float4(input.texCoord, 1.0);
}
