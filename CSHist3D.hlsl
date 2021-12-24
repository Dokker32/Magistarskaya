//Ќепридвиденное поведение на границе текстуры
cbuffer threadsParameters : register(b0)
{
	int threadsWidth;
	int threadsHeight;
	int threadsLength;
	float padding;
};

cbuffer TextureParameters : register(b1)
{
	float minScalarValue;
	float maxScalarValue;
	float minFirstDerivValue;
	float maxFirstDerivValue;
	float minSecondDerivValue;
	float maxSecondDerivValue;
	float outputTextureSize;
	float padding2;
};

RWTexture3D<uint> histTexture : register(u0);
Texture3D densityTexture : register (t0);
Texture3D gradientFirstMagnitudeTex : register (t1);
Texture3D gradientSecondMagnitudeTex : register (t2);
SamplerState mainSampler : register	(s0);

//trash
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	int binNum = outputTextureSize;
	if (dispatchThreadID.x == 0 || dispatchThreadID.x == (threadsWidth -1)) return;
	if (dispatchThreadID.y == 0 || dispatchThreadID.y == (threadsHeight -1)) return;
	if (dispatchThreadID.z == 0 || dispatchThreadID.z == (threadsLength -1)) return;
	index.x = dispatchThreadID.x / (float)(threadsWidth) + 0.5 / (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)(threadsHeight) + 0.5 / (float)threadsHeight;
	index.z = dispatchThreadID.z / (float)(threadsLength) + 0.5 / (float)threadsLength;
	float FDStep = (maxFirstDerivValue - minFirstDerivValue) / binNum;
	float SDStep = (maxSecondDerivValue - minSecondDerivValue) / binNum;
	float scalarStep = (maxScalarValue - minScalarValue) / binNum;


	float FDValue = gradientFirstMagnitudeTex.SampleLevel(mainSampler, index, 0);
	float SDValue = gradientSecondMagnitudeTex.SampleLevel(mainSampler, index, 0);
	float scalarValue = densityTexture.SampleLevel(mainSampler, index, 0);

	SDValue = SDValue - minSecondDerivValue;

	int FDBin = int(FDValue / FDStep);
	int SDBin = int(SDValue/SDStep);
	int scalarBin = scalarValue/ scalarStep;

	SDBin = binNum - SDBin; //turn Y

	InterlockedAdd(histTexture[int3(scalarBin, SDBin, FDBin)], 1);

}
