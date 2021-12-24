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
	float outputTextureSize;
	float3 padding2;
};

RWTexture2D<uint> histTexture : register(u0);
Texture3D densityTexture : register (t0);
Texture3D gradientFirstMagnitudeTex : register (t1);
SamplerState mainSampler : register	(s0);


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
	float scalarStep = (maxScalarValue - minScalarValue) / binNum;

	float FDValue = gradientFirstMagnitudeTex.SampleLevel(mainSampler, index, 0);
	float scalarValue = densityTexture.SampleLevel(mainSampler, index, 0);

	FDValue = FDValue - minFirstDerivValue;

	int FDBin = int(FDValue / FDStep);
	int scalarBin = scalarValue/ scalarStep;

	FDBin = binNum - FDBin; //turn Y

	InterlockedAdd(histTexture[int2(scalarBin, FDBin)], 1);

}
