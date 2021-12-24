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
Texture2D posFront : register (t1);
Texture2D posBack : register (t2);
Texture2D<float4> transferFunctionTexture : register (t3);
Texture3D gradMRITexture : register (t4);
SamplerState mainSampler : register	(s0);
SamplerState notInterSampler : register	(s1);
RWTexture2D<float4> outputTexture : register(u0);
RWTexture1D<float4> electrodeTexture : register (u1);


float3 GetGradient(float3 texPos);



[numthreads(32, 32, zNumThreads)]
void mainN1(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	float boneAlpha = 0.5f;
	float meatAlpha = 0.1f;
	float skinAlpha = 0.1f;
	float lungsAlpha = 0.1f;
	float closesAlpha = 0.05f;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100) {
			resultColor += alpha * float3(0.92f, 0.89f, 0.84f) * boneAlpha;
			alpha *= 1 - boneAlpha;
		}
		if (tmpValue > 900 && tmpValue < 1100) {
			resultColor += alpha * float3(0.97f, 0.56f, 0.43f) * (meatAlpha * 0.3f + meatAlpha * (1100 - tmpValue) / 200.0f / 10.0f * 7.0f);
			alpha *= 1 - meatAlpha * 0.3f - meatAlpha * (meatAlpha * 0.5f + (1100 - tmpValue) / 200.0f / 10.0f * 7.0f);
		}
		if (tmpValue > 300 && tmpValue < 900) {
			resultColor += alpha * float3(0.875f, 0.867f, 0.796f) * (skinAlpha * 0.3f + skinAlpha * (900 - tmpValue) / 600 / 10.0f * 7.0f);
			alpha *= 1 - (skinAlpha * 0.3f + skinAlpha * (900 - tmpValue) / 600 / 10.0f * 7.0f);
		}
		if (tmpValue > 200 && tmpValue < 300) {
			resultColor += alpha * float3(0.175f, 0.867f, 0.196f) * (lungsAlpha * 0.3f + lungsAlpha * (300 - tmpValue) / 100 / 10.0f * 7.0f);
			alpha *= 1 - (lungsAlpha * 0.3f + lungsAlpha * (300 - tmpValue) / 100 / 10.0f * 7.0f);
		}

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}




[numthreads(32, 32, zNumThreads)]
void main0(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize/xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	float boneAlpha = 0.5f;
	float meatAlpha = 0.03f;
	float skinAlpha = 0.05f;
	float lungsAlpha = 0.06f;
	float closesAlpha = 0.05f;
	//loop
	for (int i = 0; i < numSteps+1; ++i) {		
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100) { 
			resultColor += alpha * float3(0.92f, 0.89f, 0.84f) * boneAlpha;
			alpha *= 1 - boneAlpha;			
		}
		if (tmpValue > 900 && tmpValue < 1100) {
			resultColor += alpha * float3(0.97f, 0.56f, 0.43f)* meatAlpha;
			alpha *= 1 - meatAlpha;
		}
		if (tmpValue > 300 && tmpValue < 900) {
			resultColor += alpha * float3(0.875f, 0.867f, 0.796f)* skinAlpha;
			alpha *= 1 - skinAlpha;
		}
		if (tmpValue > 200 && tmpValue < 300) {
			resultColor += alpha * float3(0.175f, 0.867f, 0.196f) * lungsAlpha;
			alpha *= 1 - lungsAlpha;
		}
		
		currentTexCoord += rayDirection/numSteps;
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
	int numSteps = 230;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);
	float alpha = 0.f;

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {

		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		alpha += tmpValue; //0 if idle ray
		if (tmpValue > 1100)resultColor += float3(tmpValue, tmpValue * 0.3, tmpValue * 0.3);
		if (tmpValue > 400 && tmpValue < 800) resultColor += float3(tmpValue * 0.3, tmpValue, tmpValue) * 4;
		if (tmpValue > 50 && tmpValue < 300) resultColor += float3(tmpValue * 0.0, tmpValue * 0.0, tmpValue) * 16;
		currentTexCoord += rayDirection / numSteps;



	}

	resultColor /= numThreadsAlongZ * 20.0f;
	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 0.2);

	alpha = min(1, resultColor);

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}


//back to front bad 3

[numthreads(32, 32, zNumThreads)]
void main3(uint3 dispatchThreadID : SV_DispatchThreadID) {

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
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = startPos - endPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = endPos;
	float alpha = 1.0f;

	float boneAlpha = 0.5f;
	float skinAlpha = 0.03f;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100) {
			resultColor += alpha * float3(0.9f, 0.3f, 0.3f);
			alpha *= 1 - boneAlpha;
		}
		if (tmpValue > 900 && tmpValue < 1100) {
			resultColor += alpha * float3(0.3f, 0.3f, 0.9f);
			alpha *= 1 - skinAlpha;
		}
		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}


//render texture coord 4
[numthreads(32, 32, zNumThreads)]
void main4(uint3 dispatchThreadID : SV_DispatchThreadID) {

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
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 100 ) {
			//resultColor = GetGradient(currentTexCoord);
			resultColor = currentTexCoord;
			i = numSteps + 1;
		}
		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, 1.0f);
}


//data projection sample 5
[numthreads(32, 32, zNumThreads)]
void main5(uint3 dispatchThreadID : SV_DispatchThreadID) {
	float3 sensorPos = float3(0.62f, 0.4f, 0.625f);


	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 150;

	//output
	float3 resultColor = float3(1.f, 1.f, 1.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100) {
			resultColor = currentTexCoord;
			alpha = 0.0f;
			i = numSteps + 1;
			if (length(currentTexCoord - sensorPos) < 0.05f) {
				resultColor = float3(0.9f, 0.0f, 0.0f);
				alpha = 0.0f;
			}
		}
		
		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}


//shading
float3 GetGradient(float3 texPos) {
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
	for (int i = 1; i < numSteps+1; ++i) {
		tmpTexPosition = texPos;
		tmpTexPosition.x += i * step.x;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.x -= 2 * i * step.x;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.x += (pValue - nValue) / (2 * i * xMRITextureSize / xMRITextureSize);
		//y
		tmpTexPosition = texPos;
		tmpTexPosition.y += i * step.y;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.y -= 2 * i * step.y;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.y += (pValue - nValue) / (2 * i * xMRITextureSize / yMRITextureSize);
		//z
		tmpTexPosition = texPos;
		tmpTexPosition.z += i * step.z;
		//pValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		pValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		tmpTexPosition.z -= 2 * i * step.z;
		//nValue = brainTexture.Sample(mainSampler, tmpTexPosition).x;
		nValue = MRITexture.SampleLevel(mainSampler, tmpTexPosition, 0).x;
		gradient.z += (pValue - nValue) / (2 * i * xMRITextureSize / zMRITextureSize);
	}

	return normalize(gradient);
}

//shading 6
[numthreads(32, 32, zNumThreads)]
void main6(uint3 dispatchThreadID : SV_DispatchThreadID) {

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
		if (tmpValue > 1100) {
			float3 lightDir = normalize(lightPos - currentTexCoord);
			resultColor = GetGradient(currentTexCoord);
			float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			resultColor = dif * float3(0.92f, 0.89f, 0.84f);
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			resultColor += dif * float3(0.92f, 0.89f, 0.84f);
			i = numSteps + 1;
			alpha = 0.0f;
		}
		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}



//boundraies 7
[numthreads(32, 32, zNumThreads)]
void main7(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;
	bool is1 = false;
	bool is2 = false;
	bool is3 = false;

	float boneAlpha = 0.5f;
	float meatAlpha = 0.03f;
	float skinAlpha = 0.1f;
	float closesAlpha = 0.05f;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100 && is1 == false) {
			resultColor += alpha * float3(0.92f, 0.89f, 0.84f);
			alpha *= 1 - boneAlpha;
			is1 = true;
		}
		if (tmpValue > 900 && tmpValue < 1200 && is2 == false) {
			resultColor += alpha * float3(0.97f, 0.56f, 0.43f);
			alpha *= 1 - meatAlpha;
			is2 = true;
		}
		if (tmpValue > 300 && tmpValue < 900 && is3 == false) {
			resultColor += alpha * float3(0.875f, 0.867f, 0.796f);
			alpha *= 1 - skinAlpha;
			is3 = true;
		}

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}


//boundraies with shading 8
[numthreads(32, 32, zNumThreads)]
void main8(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

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
	bool is1 = false;
	bool is2 = false;
	bool is3 = false;

	float boneAlpha = 0.5f;
	float meatAlpha = 0.5f;
	float skinAlpha = 0.5f;
	float closesAlpha = 0.05f;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if (tmpValue > 1100 && is1 == false) {
			float3 lightDir = normalize(lightPos - currentTexCoord);			
			float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			resultColor += alpha * float3(0.92f, 0.89f, 0.84f) * dif;
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			//resultColor += alpha * float3(0.92f, 0.89f, 0.84f)*dif;
			alpha *= 1 - boneAlpha;
			is1 = true;
		}
		if (tmpValue > 900 && tmpValue < 1200 && is2 == false) {

			float3 lightDir = normalize(lightPos - currentTexCoord);
			float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			resultColor += alpha * float3(0.97f, 0.56f, 0.43f) * dif;
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			//resultColor += alpha * float3(0.97f, 0.56f, 0.43f) * dif;
			alpha *= 1 - meatAlpha;
			is2 = true;
		}
		if (tmpValue > 300 && tmpValue < 900 && is3 == false) {

			float3 lightDir = normalize(lightPos - currentTexCoord);
			float dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			resultColor += alpha * float3(0.875f, 0.867f, 0.796f) * dif;
			lightDir = normalize(lightPos2 - currentTexCoord);
			dif = max(dot(GetGradient(currentTexCoord), lightDir), 0.0f);
			//resultColor += alpha * float3(0.875f, 0.867f, 0.796f) * dif;
			alpha *= 1 - skinAlpha;
			is3 = true;
		}

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}


//dif detector 9


[numthreads(32, 32, zNumThreads)]
void main9(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float2 outputTexturePosition = float2(dispatchThreadID.x, dispatchThreadID.y);
	outputTexturePosition.x /= (float)xResultTextureSize;
	outputTexturePosition.y /= (float)yResultTextureSize;
	int numThreadsAlongZ = (int)(zMRITextureSize / zNumThreads);

	//parameters of alghoritm
	int numSteps = 200;

	//output
	float3 resultColor = float3(0.f, 0.f, 0.f);

	//inicialization
	float3 startPos = posFront.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	float3 rayDirection = endPos - startPos;
	rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;

	float boneAlpha = 0.3f;
	float meatAlpha = 0.6f;
	float skinAlpha = 0.1f;
	float lungsAlpha = 0.1f;
	float closesAlpha = 0.05f;

	float prevV = 0;
	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);
		if ( tmpValue > 200 && !(tmpValue>prevV*0.8f && tmpValue < prevV * 1.2f)) {
			if (tmpValue > 1100) {
				resultColor += alpha * float3(0.92f, 0.89f, 0.84f) * boneAlpha;
				alpha *= 1 - boneAlpha;
			}
			if (tmpValue > 900 && tmpValue < 1100) {
				resultColor += alpha * float3(0.97f, 0.56f, 0.43f) * meatAlpha;
				alpha *= 1 - meatAlpha;
			}
			if (tmpValue > 300 && tmpValue < 900) {
				resultColor += alpha * float3(0.875f, 0.867f, 0.796f) * skinAlpha;
				alpha *= 1 - skinAlpha;
			}
			if (tmpValue > 200 && tmpValue < 300) {
				resultColor += alpha * float3(0.175f, 0.867f, 0.196f) * lungsAlpha;
				alpha *= 1 - lungsAlpha;
			}
			prevV = tmpValue;
		}

		currentTexCoord += rayDirection / numSteps;
	}

	//tonal comression
	resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}

//shading any 10
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
	startPos.y *= xMRITextureSize/ yMRITextureSize;
	startPos.z *= 1.3;
	float3 endPos = posBack.SampleLevel(notInterSampler, outputTexturePosition, 0);
	endPos.y *= xMRITextureSize / yMRITextureSize;
	endPos.z *= 1.3;
	float3 rayDirection = endPos - startPos;
	//rayDirection.z = rayDirection.z * zMRITextureSize / xMRITextureSize;
	float3 currentTexCoord = startPos;
	float alpha = 1.0f;
	float3 lightPos = float3(10.0f, 2.0f, 2.0f);
	float3 lightPos2 = float3(-10.0f, 2.0f, 2.0f);
	float lightIntensivity = 80.0f;

	//loop
	for (int i = 0; i < numSteps + 1; ++i) {
		float tmpValue = MRITexture.SampleLevel(mainSampler, currentTexCoord, 0);		
		if (tmpValue > surfaceThresholdValue) {
			if (UpdateElectrode >= 0) {
				if (int(dispatchThreadID.x) == int(mousePosX) && int(dispatchThreadID.y) == int(mousePosY)) {
					electrodeTexture[int(UpdateElectrode)] = float4(currentTexCoord, electrodeTexture[int(UpdateElectrode)].w);					
				}
				float3 sensorPos = electrodeTexture[0].xyz;
				if (length(currentTexCoord - sensorPos) < 0.01f) {
					resultColor = resultColor = float3(0.0f, 0.9f, 0.0f);
					alpha = 0.0f;
					i = numSteps + 1;
				}
			}
			for (int j = 1; j < numElectrodes; ++j) {
				float3 sensorPos = electrodeTexture[j].xyz;
				if (length(currentTexCoord - sensorPos) < 0.015f) {
					resultColor = float3(0.9f, 0.1f, 0.1f);
					if (j==selectedElectrode) resultColor = float3(0.5f, 0.1f, 0.1f);
					alpha = 0.0f;
					i = numSteps + 1;
				}
			}
	
			if (alpha != 0.0f){
				float3 grad = normalize(gradMRITexture.SampleLevel(mainSampler, currentTexCoord, 0));
				float3 lightDir = normalize(lightPos - currentTexCoord);
				float dif = max(dot(grad, lightDir), 0.0f) * lightIntensivity / ((lightPos - currentTexCoord) * (lightPos - currentTexCoord));
				resultColor = dif * float3(0.92f, 0.89f, 0.84f);
				lightDir = normalize(lightPos2 - currentTexCoord);
				dif = max(dot(grad, lightDir), 0.0f) * lightIntensivity / ((lightPos2 - currentTexCoord) * (lightPos2 - currentTexCoord));
				resultColor += dif * float3(0.92f, 0.89f, 0.84f) + float3(0.92f, 0.89f, 0.84f) * 0.05f;
				//if (TrackingElectrode >= 0) resultColor += dif * float3(0.92f, 0.0f, 0.0f) + float3(0.92f, 0.09f, 0.0f) * 0.05f;
				i = numSteps + 1;
				alpha = 0.0f;
			}
		}
		currentTexCoord += rayDirection / numSteps;

	}

	//tonal comression
	//resultColor = float3(1.0, 1.0, 1.0) - exp(-resultColor * 1.0);
	alpha = 1 - alpha;

	outputTexture[dispatchThreadID.xy] = float4(resultColor, alpha);
}
