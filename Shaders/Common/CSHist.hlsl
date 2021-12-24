//Непридвиденное поведение на границе текстуры
cbuffer dicomParametersForShaders : register(b0)
{
	int threadsWidth;
	int threadsHeight;
	int threadsLength;
	float padding;
};

cbuffer parametersForShaders : register(b1)
{
	float maxFD;
	float maxSD;
	float minSD;
	float maxScalar;
};

Texture3D gradientMagnitudeTex : register (t1);
Texture3D brainTexture : register (t0);
RWTexture2D<uint> histTexture : register(u0);
SamplerState mainSampler : register	(s0);

//trash
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	if (dispatchThreadID.x == 0 || dispatchThreadID.x == (threadsWidth-1)) return;
	if (dispatchThreadID.y == 0 || dispatchThreadID.y == (threadsHeight-1)) return;
	if (dispatchThreadID.z == 0 || dispatchThreadID.z == (threadsLength-1)) return;
	index.x = dispatchThreadID.x / (float)(threadsWidth-1);
	index.y = dispatchThreadID.y / (float)(threadsHeight-1);
	index.z = dispatchThreadID.z / (float)(threadsLength-1);
	float fDStep = (maxFD-minSD) / 256;
	float scalarStep = maxScalar / 256;
	float fDValue = gradientMagnitudeTex.SampleLevel(mainSampler, index, 0);
	//if (fDValue < maxFD * 0.01 ) fDValue = 0;
	float SValue = brainTexture.SampleLevel(mainSampler, index, 0);
	int scalarBin = SValue/ scalarStep;
	//переворачиваем, т.к. пространство текстуры по y иде сверху вниз
	int fDBin = 128-int(fDValue / fDStep);
	if (SValue > maxSD * 0.01) InterlockedAdd(histTexture[int2(scalarBin, fDBin)], 1);

}
