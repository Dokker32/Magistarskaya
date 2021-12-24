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



float4 GetGradientAt(float3 index);

[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 index;
	index.x = dispatchThreadID.x / (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)threadsHeight;
	index.z= dispatchThreadID.z / (float)threadsLength;
	outputGradText[dispatchThreadID.xyz] = GetGradientAt(index);//float4(1.0f, 1.0f, 1.0f, 1)/288/288/117* dispatchThreadID.x* dispatchThreadID.y* dispatchThreadID.z;

	//GetGradientValueAt(index);
}

float4 GetGradientAt(float3 index) {
	float3 xShift = float3(1.0f / threadsWidth,0,0);
	float3 yShift = float3(0, 1.0f / threadsHeight, 0);
	float3 zShift = float3(0, 0, 1.0f / threadsLength);
	
	float xNegativeValue = brainTexture.SampleLevel(mainSampler, index - xShift, 0);
	float xPositiveValue = brainTexture.SampleLevel(mainSampler, index + xShift, 0);
	float yNegativeValue = brainTexture.SampleLevel(mainSampler, index - yShift, 0);
	float yPositiveValue = brainTexture.SampleLevel(mainSampler, index + yShift, 0);
	float zNegativeValue = brainTexture.SampleLevel(mainSampler, index - zShift, 0);
	float zPositiveValue = brainTexture.SampleLevel(mainSampler, index + zShift, 0);
	float3 gradient;
	gradient.x = xPositiveValue - xNegativeValue;
	gradient.y = yPositiveValue - yNegativeValue;
	gradient.z = zPositiveValue - zNegativeValue;

	//gradient.y /= threadsWidth / threadsHeight;
	//gradient.z /= threadsWidth / threadsLength;
	return float4(gradient, 1.0f); //DirectX11 supports only rgba, not rgb
}