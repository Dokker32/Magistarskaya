#pragma once

struct texture3DDesc {
	unsigned int columns;
	unsigned int rows;
	unsigned int slices;
	unsigned int channels;
	unsigned int bitsAllocated;
	unsigned int bitsActually;
	unsigned int bitsHight;
	float zCompression;
};

struct IESquad {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 posTex;
};


struct dicomParametersForShaders {
	float zCompression;
	
	float textureWidth;
	float textureHeight;
	float textureSlices;
	float shadingSmoothing;
	
	
	DirectX::XMFLOAT3 padding;
};


enum CurrentSwapChain { MAIN, TRANSFER_FUNCTION_2D, TRANSFER_FUNCTION_3D };

struct MPoint {
	float x = 0;
	float y = 0;

	MPoint(float x, float y) : x(x), y(y){}
};

struct TF2DPoint {
	float x;
	float y;
	int tissueId;
	int localId;
	Windows::UI::Color color;
};

