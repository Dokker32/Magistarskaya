#define zNumThreads 1

cbuffer dicomParametersForShaders : register(b0)
{
	unsigned int mousePosX;
	unsigned int mousePosY;
	unsigned int xResultTextureSize;
	unsigned int yResultTextureSize;
	unsigned int xMRITextureSize;
	unsigned int yMRITextureSize;
	unsigned int zMRITextureSize;
	float maxScalarValue;
	float maxGradMagn;
	unsigned int surfaceThresholdValue;
	unsigned int brainThresholdValue;
	float numElectrodes;
	int UpdateElectrode;
	int selectedElectrode;
	float eegActivityNormalization;
	float eegActivityExp;
};

Texture3D MRITexture : register (t0);
Texture3D MRISecondTexture : register (t6);
Texture2D posFront : register (t1);
Texture2D posBack : register (t2);
Texture2D<float4> transferFunctionTexture : register (t3);
SamplerState mainSampler : register	(s0);
SamplerState notInterSampler : register	(s1);
RWTexture2D<float4> outputTexture : register(u0);
RWTexture1D<float4> electrodeTexture : register (u1);

float3 GetGradient(float3 texPos);

//shading
float3 GetGradient(float3 texPos, texture3D inputTexture) {
	float3 gradient = 0;
	float3 tmpTexPosition;
	float nValue;
	float pValue;
	int numSteps = 1;
	float3 step = 1.0f;
	step.x /= xMRITextureSize;
	step.y /= yMRITextureSize;
	step.z /= zMRITextureSize;
	//x
	for (int i = 1; i < numSteps + 1; ++i) {
		tmpTexPosition = texPos;
		tmpTexPosition.x += i * step.x;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.x -= 2 * i * step.x;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.x += (pValue - nValue) / (2 * i * xMRITextureSize / xMRITextureSize);
		//y
		tmpTexPosition = texPos;
		tmpTexPosition.y += i * step.y;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.y -= 2 * i * step.y;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.y += (pValue - nValue) / (2 * i * xMRITextureSize / yMRITextureSize);
		//z
		tmpTexPosition = texPos;
		tmpTexPosition.z += i * step.z;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.z -= 2 * i * step.z;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = inputTexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.z += (pValue - nValue) / (2 * i * xMRITextureSize / zMRITextureSize);
	}

	return normalize(gradient);
}

//shading any 1
[numthreads(32, 32, zNumThreads)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(1.f, 1.f, 1.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	//startPos.y *= yMRITextureSize/ xMRITextureSize;
	startPos.z *= 1.3;
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	//endPos.y *= yMRITextureSize / xMRITextureSize;
	endPos.z *= 1.3;
	float3 rayDirection = endPos - startPos;
	//rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;
	float3 lightPos = float3(10.0f, 2.0f, 2.0f);
	float3 lightPos2 = float3(-10.0f, 2.0f, 2.0f);
	float lightIntensivity = 80.0f;

	bool is1 = false;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRISecondTexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > brainThresholdValue) {
			float3 gradient = GetGradient(currentTexCoord, MRISecondTexture);
			float3 lightDir = normalize(lightPos - currentTexCoord);			
			float dif = max(dot(-gradient, lightDir), 0.0f) * lightIntensivity / ((lightPos - currentTexCoord) * (lightPos - currentTexCoord));
			resultColor = dif * float3(0.9803f, 0.7098f, 0.6509f);
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(-gradient, lightDir), 0.0f) * lightIntensivity / ((lightPos2 - currentTexCoord) * (lightPos2 - currentTexCoord));
			resultColor += dif * float3(0.9803f, 0.7098f, 0.6509f) + float3(0.9803f, 0.7098f, 0.6509f) * 0.05f;
			i = numSteps + 1;
			alpha = 0.0f;

			int closestElectrodeId = 0;
			float minLength = 1.0f;
			for (int j = 1; j < numElectrodes; ++j) {
				float3 sensorPos = electrodeTexture[j].xyz;
				float3 sensorVal = electrodeTexture[j].w;
				if (minLength > length(currentTexCoord - sensorPos)) {
					minLength = length(currentTexCoord - sensorPos);
					closestElectrodeId = j;
				}
			}
			float3 sensorPos = electrodeTexture[closestElectrodeId].xyz;
			float3 sensorVal = electrodeTexture[closestElectrodeId].w;
			if (length(currentTexCoord - sensorPos) < 0.4f) {
				float3 color = lerp(float3(0.1f, 1.0f, 0.1f), float3(0.1f, 0.1f, 1.0f), sign(sensorVal) / 2. + 0.5);
				resultColor = lerp(color * abs(sensorVal) / 100. * eegActivityNormalization, resultColor, pow(length(currentTexCoord - sensorPos), 0.00001 * pow(10., eegActivityExp)));
			}

		}
		else{
			float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
			if (tmpValue > surfaceThresholdValue && !is1 ) {
				float3 gradient = GetGradient(currentTexCoord, MRITexture);
				float3 lightDir = normalize(lightPos - currentTexCoord);
				float dif = max(dot(-gradient, lightDir), 0.0f) * lightIntensivity / ((lightPos - currentTexCoord) * (lightPos - currentTexCoord));
				resultColor = dif * float3(0.92f, 0.89f, 0.84f);
				lightDir = normalize(lightPos2 - currentTexCoord);
				dif = max(dot(-gradient, lightDir), 0.0f) * lightIntensivity / ((lightPos2 - currentTexCoord) * (lightPos2 - currentTexCoord));
				resultColor += dif * float3(0.92f, 0.89f, 0.84f) + float3(0.92f, 0.89f, 0.84f) * 0.05f;
				//i = numSteps + 1;
				alpha *= 1 - 0.7;
				is1 = true;
			}
		}
		
		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}

// 2
[numthreads(32, 32, zNumThreads)]
void main2(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(1.f, 1.f, 1.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	//startPos.y *= yMRITextureSize/ xMRITextureSize;
	//startPos.z *= 1.8f;
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	//endPos.y *= yMRITextureSize / xMRITextureSize;
	//endPos.z *= 1.8f;
	float3 rayDirection = endPos - startPos;
	//rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;
	float3 lightPos = float3(10.0f, 2.0f, 2.0f);
	float3 lightPos2 = float3(-10.0f, 2.0f, 2.0f);
	float lightIntensivity = 80.0f;

	bool is1 = false;
	float someAlpha = 0.5f;

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {

		float tmpValue = MRISecondTexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > brainThresholdValue) {			
			float3 lightDir = normalize(lightPos - currentTexCoord);
			float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f) * lightIntensivity / ((lightPos - currentTexCoord) * (lightPos - currentTexCoord));
			resultColor = dif * float3(0.92f, 0.89f, 0.84f);
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f) * lightIntensivity / ((lightPos2 - currentTexCoord) * (lightPos2 - currentTexCoord));
			resultColor += dif * float3(0.92f, 0.89f, 0.84f) + float3(0.92f, 0.89f, 0.84f) * 0.05f;
			i = numSteps + 1;
			alpha = 0.0f;

			

		}
		else {
			float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
			if (tmpValue > surfaceThresholdValue && is1 == false) {
				float3 lightDir = normalize(lightPos - currentTexCoord);
				float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f) * lightIntensivity / ((lightPos - currentTexCoord) * (lightPos - currentTexCoord));
				resultColor = alpha * (dif * float3(0.92f, 0.89f, 0.84f));
				lightDir = normalize(lightPos2 - currentTexCoord);
				dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f) * lightIntensivity / ((lightPos2 - currentTexCoord) * (lightPos2 - currentTexCoord));
				resultColor += alpha * (dif * float3(0.92f, 0.89f, 0.84f) + float3(0.92f, 0.89f, 0.84f) * 0.05f);
				alpha *= 1 - someAlpha;
				is1 = true;
			}
		}

		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}
