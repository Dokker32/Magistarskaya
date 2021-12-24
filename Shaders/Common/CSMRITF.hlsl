#define zNumThreads 1

cbuffer dicomParametersForShaders : register(b0)
{
	unsigned int nearFarPlaneX;
	unsigned int nearFarPlaneY;
	unsigned int xResultTextureSize;
	unsigned int yResultTextureSize;
	unsigned int xMRITextureSize;
	unsigned int yMRITextureSize;
	unsigned int zMRITextureSize;
	float maxScalarValue;
	float maxGradMagn;
	float3 padding;
};

Texture3D MRITexture : register (t0);
Texture2D posFront : register (t1);
Texture2D posBack : register (t2);
Texture2D<float4> transferFunctionTexture : register (t3);
Texture3D<float4> gradTexture : register (t4);
Texture2D<uint> histTexture : register (t5);
SamplerState mainSampler : register	(s0);
SamplerState notInterSampler : register	(s1);
RWTexture2D<float4> outputTexture : register(u0);

float3 GetGradient(float3 texPos);
bool CheckForSurface(float3 texPos);

[numthreads(32, 32, zNumThreads)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 500;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	float3 lightPos = float3(10.0f, 5.0f, 1.0f);
	float3 lightPos2 = float3(-12.0f, 3.0f, 2.0f);

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1.0f) {

				float3 grad = gradTexture.SampleLevel(mainSampler, currentTexCoord, 0).xyz;
				float4 c = transferFunctionTexture.SampleLevel(mainSampler, float2(tmpValue / maxScalarValue, 1.0f - length(grad) / maxGradMagn), 0);
				float3 lightDir = normalize(lightPos - currentTexCoord);
				float dif = max(dot(normalize(grad), lightDir), 0.0f);
				lightDir = normalize(lightPos2 - currentTexCoord);
				dif += max(dot(normalize(grad), lightDir), 0.0f);
				resultColor += alpha * (c.a * (c.rgb * dif + c.rgb * 0.05f) );
				alpha *= 1 - c.a;
			
		}

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);

	alpha = 1 - alpha;


	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}

[numthreads(32, 32, zNumThreads)]
void main2(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 100;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 0.0f;
	float3 lightPos = float3(10.0f, 5.0f, 1.0f);
	float3 lightPos2 = float3(-12.0f, 3.0f, 2.0f);

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1.0f) {
			
				float3 grad = GetGradient(currentTexCoord);
				float4 c = transferFunctionTexture.SampleLevel(mainSampler, float2(tmpValue / maxScalarValue, 1.0f - length(grad) / maxGradMagn), 0);
				if (c.a > 0.0f && histTexture.Load(int3(float2(tmpValue / maxScalarValue, 1.0f - length(grad) / maxGradMagn) * 511, 0)) > 5000) {
					i = 1000;
					float3 lightDir = normalize(lightPos - currentTexCoord);
					float dif = max(dot(normalize(grad), lightDir), 0.0f);
					lightDir = normalize(lightPos2 - currentTexCoord);
					dif += max(dot(normalize(grad), lightDir), 0.0f);
					resultColor += c.rgb * dif + c.rgb * 0.01f;
					alpha = 1.0f;
				}
			
		}
		

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);



	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}

//shading
float3 GetGradient(float3 texPos) {
	float3 gradient = 0;
	float3 tmpTexPosition;
	float nValue;
	float pValue;
	int numSteps = 1;
	float3 step = 1.0f / xMRITextureSize;
	//float3 step = 1.0f/250;
	//x
	for (int i = 1; i < numSteps + 1; ++i) {
		tmpTexPosition = texPos;
		tmpTexPosition.x += i * step;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.x -= 2 * i * step;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.x += (pValue - nValue) / (2 * i);
		//y
		tmpTexPosition = texPos;
		tmpTexPosition.y += i * step;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.y -= 2 * i * step;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.y += (pValue - nValue) / (2 * i);
		//z
		tmpTexPosition = texPos;
		tmpTexPosition.z += i * (1.0f / zMRITextureSize);
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.z -= 2 * i * (1.0f / zMRITextureSize);
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.z += (pValue - nValue) / (2 * i * zMRITextureSize / xMRITextureSize);
	}

	return gradient;
}



//
bool CheckForSurface(float3 texPos) {
	float3 gradient = 0;
	float3 tmpTexPosition;
	float nValue;
	float pValue;
	int numSteps = 1;
	float3 step = 1.0f / xMRITextureSize;
	float3 grad;
	float localGrad = GetGradient(texPos);

	int numVoxSurf = 0;
	int numTrheshold = 2;
	float dotThreshold = 0.08f;
	//float3 step = 1.0f/250;
	for (int i = 1; i < numSteps + 1; ++i) {
		//x
		tmpTexPosition = texPos;
		tmpTexPosition.x += i * step;
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad)<(1.5f* length(localGrad)) && (length(grad)>0.6f*length(localGrad))) {
			++numVoxSurf;
		}
		tmpTexPosition.x -= 2 * i * step;
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad) < (1.5f * length(localGrad)) && (length(grad) > 0.6f * length(localGrad))) {
			++numVoxSurf;
		}
		//y
		tmpTexPosition = texPos;
		tmpTexPosition.y += i * step;
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad) < (1.5f * length(localGrad)) && (length(grad) > 0.6f * length(localGrad))) {
			++numVoxSurf;
		}
		tmpTexPosition.y -= 2 * i * step;
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad) < (1.5f * length(localGrad)) && (length(grad) > 0.6f * length(localGrad))) {
			++numVoxSurf;
		}
		//z
		tmpTexPosition = texPos;
		tmpTexPosition.z += i * (1.0f / zMRITextureSize);
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad) < (1.5f * length(localGrad)) && (length(grad) > 0.6f * length(localGrad))) {
			++numVoxSurf;
		}
		tmpTexPosition.z -= 2 * i * (1.0f / zMRITextureSize);
		grad = GetGradient(tmpTexPosition);
		if (dot(normalize(grad), normalize(localGrad)) > dotThreshold
			&& length(grad) < (1.5f * length(localGrad)) && (length(grad) > 0.6f * length(localGrad))) {
			++numVoxSurf;
		}
	}

	if (numVoxSurf >= numTrheshold) {
		return true;
	}
	return false;
}


//3 grad vis
[numthreads(32, 32, zNumThreads)]
void main3(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 500;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	float3 lightPos = float3(10.0f, 5.0f, 1.0f);
	float3 lightPos2 = float3(-12.0f, 3.0f, 2.0f);

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100.0f) {

			resultColor = normalize(gradTexture.SampleLevel(mainSampler, currentTexCoord, 0).xyz);
			i = numSteps + 1;

		}

		currentTexCoord += rayDirection / numSteps;
	}



	outputTexture[dispatchThreadID.xy] = float4(resultColor, 1.0f);
}