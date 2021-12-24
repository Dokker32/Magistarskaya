//Ќепридвиденное поведение на границе текстуры


cbuffer dicomParametersForShaders : register(b0)
{
	int threadsWidth;
	int threadsHeight;
	int threadsLength;
	float padding;
};

Texture3D brainTexture : register (t0);
SamplerState mainSampler : register	(s0);
RWTexture3D<float> outputScalarText : register(u0);


float GetScalarAt(float3 index);

[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	index.x = dispatchThreadID.x / (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)threadsHeight;
	index.z = dispatchThreadID.z / (float)threadsLength;
	outputScalarText[dispatchThreadID.xyz] = GetScalarAt(index);//float4(1.0f, 1.0f, 1.0f, 1)/288/288/117* dispatchThreadID.x* dispatchThreadID.y* dispatchThreadID.z;

	//GetGradientValueAt(index);
}

float GetScalarAt(float3 index) {
	float xShift = 1.0f / threadsWidth;
	float yShift = 1.0f / threadsHeight;
	float zShift = 1.0f / threadsLength;

	float scalar = 0.0f;

	for (int x = -1; x <= 0; ++x) {
		for (int y = -1; y <= 0; ++y) {
			for (int z = -1; z <= 0; ++z) {
				scalar += brainTexture.SampleLevel(mainSampler,
					float3 (index.x + (x)*xShift, index.y + (y)*yShift, index.z + (z)*zShift), 0).x;
			}
		}
	}
	// average
	scalar /= 8.0f;

	return scalar; //DirectX11 supports only rgba, not rgb
}