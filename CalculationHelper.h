#pragma once
#include "pch.h"
#include "..\Common\DirectXHelper.h"
#include <d3dcompiler.h>


//Получает на вход трехмерную текстуру. Рассчитывает значение градиента в каждом элементарном объеме и заполняет этими данными вторую текстуру такого же размера
void CalculateGradientMagnitude6(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);

void CalculateGradientMagnitude26(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);


void CalculateGradient6Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);

void CalculateGradient26Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context); 

void CalculateNodeGradient8Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);

void CalculateNodeScalar8Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);
//TODO
//Получает на вход трехмерную текстуру. Рассчитывает значение второй производной вдоль градиента в каждом элементарном объеме и заполняет этими данными вторую текстуру такого же размера
void CalculateSecondDerivativeAlongGradient(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);
//производит свертку трехмерной текстуры по ПРОИЗВОЛЬНОМУ ядру
void ApplyConvolution(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, 
	ID3D11Device* device, ID3D11DeviceContext* context, uint32_t kernelRadius, float sigma);

void createGaussKernel(uint16_t radius, float sigma, ID3D11Texture3D** outputTexture, ID3D11Device* device);

void CalculateHist3D(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context,
	int histSize);

void CalculateHist3D(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context,
	int histSize, ID3D11Texture3D** firstDerivTex, ID3D11Texture3D** secondDerivTexture);

void CalculateHist3D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2, ID3D11Texture3D* inputTexture3,
	ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize);

// Hist 2D
void CalculateHist2D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2, 
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize);

void CalculateHist2D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize,
	float* maxScalarOut, float* maxGradMagnOut);

void CalculateHist2D(ID3D11Texture3D* inputTexture,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize);

void CalculateHist2D(ID3D11Texture3D* inputTexture,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize, 
	float* maxScalarOut, float* maxGradMagnOut);

void CalculateHist2D(ID3D11Texture3D* inputTexture,	ID3D11Texture2D** outputTexture, ID3D11Device* device,
	ID3D11DeviceContext* context, int outputSize, ID3D11Texture3D** firstDerivTexture);

template <class T>
void FindMinMaxValuesInTexure3D(ID3D11Texture3D* inputTexture, ID3D11Device* device, ID3D11DeviceContext* context, T* minValue, T* maxValue);

template <class T>
void FindMinMaxValuesInTexure2D(ID3D11Texture2D* inputTexture, ID3D11Device* device, ID3D11DeviceContext* context, T* minValue, T* maxValue);

void FindSurfaces(ID3D11Texture3D* scalarTexture, ID3D11Texture3D* gradTexture, ID3D11Texture3D* outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context);

void IsSurface(float* pScalarData, DirectX::XMFLOAT4* pGradData, uint16_t* SurefaceIdData, DirectX::XMINT3* texCoord,
	float* prevScalarValue, DirectX::XMFLOAT4* prevGrad, int* width, int* height, int* depth, int* id, int* surfaceSize, 
	int* thresholdChecker);

float XMFLOAT3GetLength(DirectX::XMFLOAT3* input);

float XMFLOAT3Dot(DirectX::XMFLOAT3* input1, DirectX::XMFLOAT3* input2);

float XMFLOAT3GetLength(DirectX::XMFLOAT4* input);

float XMFLOAT3Dot(DirectX::XMFLOAT4* input1, DirectX::XMFLOAT4* input2);

DirectX::XMFLOAT3 XMFLOAT3Normalize(DirectX::XMFLOAT3* input1);

DirectX::XMFLOAT3 XMFLOAT3Normalize(DirectX::XMFLOAT4* input1);

void CheckVoxelsForNeighborhoods(std::vector<DirectX::XMINT3>* oldVoxels, std::vector<DirectX::XMINT3>* newVoxels,
	float* pScalarData, DirectX::XMFLOAT4* pGradData, bool* tmpIdData, bool* isChecked, int* surfaceSize,
	int16_t width, int16_t height, int16_t depth);