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
RWTexture3D<float4> outputGradText : register(u0);



float GetGradientMagnitudeAt(float3 index);

[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	index.x = dispatchThreadID.x / (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)threadsHeight;
	index.z= dispatchThreadID.z / (float)threadsLength;
	outputGradText[dispatchThreadID.xyz] = GetGradientMagnitudeAt(index);//float4(1.0f, 1.0f, 1.0f, 1)/288/288/117* dispatchThreadID.x* dispatchThreadID.y* dispatchThreadID.z;

	//GetGradientValueAt(index);
}

float GetGradientMagnitudeAt(float3 index) {
	float xShift = 1.0f / threadsWidth;
	float yShift = 1.0f / threadsHeight;
	float zShift = 1.0f / threadsLength;
	
	float centralValue = brainTexture.SampleLevel(mainSampler, index, 0);
	float3 gradient = float3(0.0f, 0.0f, 0.0f);

	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				float3 direction = float3(x-1, y-1, z-1);
				gradient -= direction*(centralValue - brainTexture.SampleLevel(mainSampler,
					float3 (index.x + (x - 1) * xShift, index.y + (y - 1) * yShift, index.z + (z - 1) * zShift), 0).x);
			}
		}
	}
	
	return float(gradient.x); //DirectX11 supports only rgba, not rgb
}