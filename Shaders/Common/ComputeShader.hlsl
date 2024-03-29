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
	index.z = dispatchThreadID.z / (float)threadsLength;
	float gradient = GetGradientMagnitudeAt(index);
	outputFirstDeriv[dispatchThreadID.xyz] = gradient;//float4(1.0f, 1.0f, 1.0f, 1)/288/288/117* dispatchThreadID.x* dispatchThreadID.y* dispatchThreadID.z;
	outputSecondDeriv[dispatchThreadID.xyz] = float4(1.0f, 1.0f, 1.0f, 1);
	//GetGradientValueAt(index);
}

float GetGradientMagnitudeAt(float3 index) {
	float3 xShift = float3(1.0f / threadsWidth,0,0);
	float3 yShift = float3(0, 1.0f / threadsHeight, 0);
	float3 zShift = float3(0, 0, 1.0f / threadsLength);
	
	float centralValue = brainTexture.SampleLevel(mainSampler, index, 0);
	float xNegativeValue = brainTexture.SampleLevel(mainSampler, index - xShift, 0) - centralValue;
	float xPositiveValue = brainTexture.SampleLevel(mainSampler, index + xShift, 0) - centralValue;
	float yNegativeValue = brainTexture.SampleLevel(mainSampler, index - yShift, 0) - centralValue;
	float yPositiveValue = brainTexture.SampleLevel(mainSampler, index + yShift, 0) - centralValue;
	float zNegativeValue = brainTexture.SampleLevel(mainSampler, index - zShift, 0) - centralValue;
	float zPositiveValue = brainTexture.SampleLevel(mainSampler, index + zShift, 0) - centralValue;
	float3 gradient;
	gradient.x = (xPositiveValue - xNegativeValue);
	gradient.y = (yPositiveValue - yNegativeValue);
	gradient.z = (zPositiveValue - zNegativeValue)/ threadsWidth* threadsLength;
	return length(gradient);
}