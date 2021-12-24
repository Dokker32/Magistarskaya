//Ќепридвиденное поведение на границе текстуры


cbuffer dicomParametersForShaders : register(b0)
{
	unsigned int threadsWidth;
	unsigned int threadsHeight;
	unsigned int threadsLength;
	float padding;
};

Texture3D brainTexture : register (t0);
SamplerState mainSampler : register	(s0);
RWTexture3D<float> ouputTexture : register(u0);



float1x3 GetGradientAt(float3 texCoord);
float3x3 GetHessianAt(float3 texCoord);
float GetDeltaXAt(float3 index);
float GetDeltaYAt(float3 texCoord);
float GetDeltaZAt(float3 texCoord);
float3 IndexToTexCoord(float3 index);
float m_length(float1x3 input);

[numthreads(32, 32, 1)]
void main(uint3 threadIDInGroup : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 texCoord;
	texCoord.x = dispatchThreadID.x / ((float)threadsWidth) + 0.5/ ((float)threadsWidth);
	texCoord.y = dispatchThreadID.y / ((float)threadsHeight) + 0.5 / ((float)threadsHeight);
	texCoord.z = dispatchThreadID.z / ((float)threadsLength) + 0.5 / ((float)threadsLength);

	float3x3 hessian = GetHessianAt(texCoord);
	float1x3 grad = GetGradientAt(texCoord);
	float value = (1 / pow(m_length(grad), 2)) * mul(mul(transpose(grad), hessian), grad);
	ouputTexture[dispatchThreadID] = 0;
	if (!isnan(value)) ouputTexture[dispatchThreadID] = value;

}



float3x3 GetHessianAt(float3 texCoord) {
	//xx,xy,xz; yx,yy,yz;zx,zy,zz
	float3 xShift = float3(1.0f / threadsWidth, 0, 0);
	float3 yShift = float3(0, 1.0f / threadsHeight, 0);
	float3 zShift = float3(0, 0, 1.0f / threadsLength);

	float3x3 hessian;

	hessian._11 = GetDeltaXAt(texCoord + xShift) - GetDeltaXAt(texCoord - xShift);
	hessian._12 = GetDeltaXAt(texCoord + yShift) - GetDeltaXAt(texCoord - yShift);
	hessian._13 = GetDeltaXAt(texCoord + zShift) - GetDeltaXAt(texCoord - zShift);

	hessian._21 = GetDeltaYAt(texCoord + xShift) - GetDeltaXAt(texCoord - xShift);
	hessian._22 = GetDeltaYAt(texCoord + yShift) - GetDeltaXAt(texCoord - yShift);
	hessian._23 = GetDeltaYAt(texCoord + zShift) - GetDeltaXAt(texCoord - zShift);

	hessian._31 = GetDeltaZAt(texCoord + xShift) - GetDeltaXAt(texCoord - xShift);
	hessian._32 = GetDeltaZAt(texCoord + yShift) - GetDeltaXAt(texCoord - yShift);
	hessian._33 = GetDeltaZAt(texCoord + zShift) - GetDeltaXAt(texCoord - zShift);

	return hessian;
}


float GetDeltaXAt(float3 texCoord) {
	float deltaValue;
	float3 xShift = float3(1.0f / threadsWidth, 0, 0);

	float xNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - xShift, 0);
	float xPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + xShift, 0);
	deltaValue = xPositiveValue - xNegativeValue;

	return deltaValue;
}

float GetDeltaYAt(float3 texCoord) {
	float deltaValue;
	float3 yShift = float3(0, 1.0f / threadsHeight, 0);

	float yNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - yShift, 0);
	float yPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + yShift, 0);
	deltaValue = yPositiveValue - yNegativeValue;

	return deltaValue;

}

float GetDeltaZAt(float3 texCoord) {
	float deltaValue;
	float3 zShift = float3(0, 0, 1.0f / threadsWidth);

	float zNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - zShift, 0);
	float zPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + zShift, 0);
	deltaValue = zPositiveValue - zNegativeValue;

	return deltaValue;

}


float3 IndexToTexCoord(float3 index) {
	index.x /= (threadsWidth - 1);
	index.y /= (threadsHeight - 1);
	index.z /= (threadsLength - 1);
	return index;
}

float1x3 GetGradientAt(float3 texCoord) {
	float3 xShift = float3(1.0f / threadsWidth , 0, 0);
	float3 yShift = float3(0, 1.0f / threadsHeight , 0);
	float3 zShift = float3(0, 0, 1.0f / threadsLength );


	float xNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - xShift, 0);
	float xPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + xShift, 0);
	float yNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - yShift, 0);
	float yPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + yShift, 0);
	float zNegativeValue = brainTexture.SampleLevel(mainSampler, texCoord - zShift, 0);
	float zPositiveValue = brainTexture.SampleLevel(mainSampler, texCoord + zShift, 0);
	float1x3 gradient;
	gradient._11 = xPositiveValue - xNegativeValue;
	gradient._12 = yPositiveValue - yNegativeValue;
	gradient._13 = zPositiveValue - zNegativeValue;
	return gradient;
}

float m_length(float1x3 input) {
	float3 v;
	v.x = input._11;
	v.y = input._12;
	v.z = input._13;

	return length(v);

}