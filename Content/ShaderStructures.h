#pragma once

namespace MRIRenderRCT
{
	// Буфер констант, используемый для отправки матриц MVP в шейдер вершин.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Используется для отправки данных отдельных вершин в шейдер вершин.
	struct VertexPositionColorTexcoord
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 texCoord;
	};

	struct VertexPositionTexcoord
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 texCoord;
	};

	// Используется для отправки данных отдельных вершин в шейдер вершин.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoord;
	};

	struct MRIParamStruct {
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
		float eegActivityNormalization = 1;
		float eegActivityExp;
	};
}