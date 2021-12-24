//�������������� ��������� �� ������� ��������


cbuffer dicomParametersForShaders : register(b0)
{
	int threadsWidth;
	int threadsHeight;
	int threadsLength;
	float padding;
};

Texture3D brainTexture : register (t0);
SamplerState mainSampler : register	(s0);
RWTexture3D<float> outputFirstDeriv : register(u0);
RWTexture3D<float> outputSecondDeriv : register(u1);
RWBuffer<float> maxGradMagnitude : register(u2);


float GetGradientMagnitudeAt(float3 index);

[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	index.x = dispatchThreadID.x / (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)threadsHeight;
	index.z= dispatchThreadID.z / (float)threadsLength;
	float gradient = GetGradientMagnitudeAt(index);
	outputFirstDeriv[dispatchThreadID.xyz] = gradient;//float4(1.0f, 1.0f, 1.0f, 1)/288/288/117* dispatchThreadID.x* dispatchThreadID.y* dispatchThreadID.z;
	outputSecondDeriv[dispatchThreadID.xyz] = float4(1.0f, 1.0f, 1.0f, 1);
	//GetGradientValueAt(index);
}

float GetGradientMagnitudeAt(float3 index) {
	float xShift = 1.0f / threadsWidth;
	float yShift = 1.0f / threadsHeight;
	float zShift = 1.0f / threadsLength;

	float centralValue = brainTexture.SampleLevel(mainSampler, index, 0);
	float3 gradient = 0.0f;

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			for (int z = -1; z <= 1; ++z) {
				float3 direction = float3(x, y, z);
				gradient -= direction * (centralValue - brainTexture.SampleLevel(mainSampler,
					float3 (index.x + (x)*xShift, index.y + (y)*yShift, index.z + (z)*zShift), 0).x);
			}
		}
	}

	return length(gradient); //DirectX11 supports only rgba, not rgb
}