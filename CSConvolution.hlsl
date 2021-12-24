//Ќепридвиденное поведение на границе текстуры


cbuffer dicomParametersForShaders : register(b0)
{
	unsigned int threadsWidth;
	unsigned int threadsHeight;
	unsigned int threadsLength;
	unsigned int kernelRadius;
};


Texture3D inputTexture : register (t0);
Texture3D kernel : register (t7);
SamplerState mainSampler : register	(s0);
RWTexture3D<float> outputTexture : register(u0);


float calculateValueAt(float3 texCoord);

[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {

	float3 index;
	index.x = dispatchThreadID.x / (float)threadsWidth + 0.5/ (float)threadsWidth;
	index.y = dispatchThreadID.y / (float)threadsHeight + 0.5/ (float)threadsHeight;
	index.z = dispatchThreadID.z / (float)threadsLength + 0.5/ (float)threadsLength;
	unsigned int kernelSize = kernelRadius * 2 - 1;
	float value = 0;

	for (unsigned int x = 0; x < kernelSize; ++x) {
		for (unsigned int y = 0; y < kernelSize; ++y) {
			for (unsigned int z = 0; z < kernelSize; ++z) {
				float3 texCoord = index;				
				texCoord.x -= (1 / (float)threadsWidth) * ((float)kernelRadius - (x + 1));
				texCoord.y -= (1 / (float)threadsHeight) * ((float)kernelRadius - (y + 1));
				texCoord.z -= (1 / (float)threadsLength) * ((float)kernelRadius - (z + 1));
				float3 kernelCoord = float3(x/ (kernelSize-1), y/ (kernelSize-1), z/ (kernelSize-1));
				kernelCoord = float3(((float)x) / (kernelSize-1),((float)y) / (kernelSize-1),((float)z) / (kernelSize-1));
				value += inputTexture.SampleLevel(mainSampler, texCoord, 0).x *kernel.SampleLevel(mainSampler, kernelCoord, 0).x;
			}
		}
	}

	outputTexture[dispatchThreadID.xyz] = value;
}


