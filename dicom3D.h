	#pragma once

#include <iostream>
#include <fstream>

#include "helper.h"
#include "Common/DeviceResources.h"
#include "CalculationHelper.h"

using namespace std;

class Dicom3D {
public:
	Dicom3D(string fileName) {
		this->fileName = fileName;
	}
	~Dicom3D();

	void OpenFile();
	void CloseFile();
	void ReadDesc();
	texture3DDesc GetTexDesc();
	void CreateID3D11ShaderResourceView(
		const std::shared_ptr<DX::DeviceResources>& deviceResources,
		  ID3D11ShaderResourceView**, float** histoData, int& numColumns, float thresholdValue
	);
	void GetRawData(float** outputData, int** outputWidth, int** outputHeight, int** outputSlices);
	unsigned int GetMaxValue() {
		return this->maxValue;
	}

private:
	bool CompareTag(unsigned char* tag, unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	//ищет тег в данных после указателя. Изменяет входящий указатель на указатель на данные, стоящие сразу после тега.
	void FindTag(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	//сдвиг на value байт
	void ShiftBy(long value);
	//dont use
	int GetTagValue(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	int GetTagValue2(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	float GetXZRatio();
	//gaussian convolution
	bool ConvolutionG3D();
	//calculate derivaties gistogramm, creating resources for it
	bool CalculateDerivatiesGist();


	//fields
	std::string fileName;
	texture3DDesc texDesc;
	int maxGradientMagnitude = 0;
	FILE* pFile;
	float* outputData;
	unsigned int maxValue = 0;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader2;
	
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavDerivatiesGist;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavHistTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvTexture3D;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvHistTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvSecondDerivatiesTexture;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				MRIDescDataCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				bDerivatiesGist;

	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				HistTexture;
	
	std::shared_ptr<DX::DeviceResources>				deviceResources;

	bool isFileOpened = false;
	bool isTextureLoaded = false;
};

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------

class DicomSlice3D {
public:
	DicomSlice3D(std::vector<byte>* inputData) {
		this->dataVec = inputData;
	}
	~DicomSlice3D();

	void GetData(std::vector<byte>** outputVector, int* position);
	void GetDataSecond(std::vector<byte>** outputVector, int* position);
	void GetDataSize(int* size);
	void ReadDesc(int numFiles);
	texture3DDesc GetTexDesc();

private:
	bool CompareTag(unsigned char* tag, unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	//ищет тег в данных после указателя. Изменяет входящий указатель на указатель на данные, стоящие сразу после тега.
	void FindTag(int* pos,unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	//сдвиг на value байт
	int GetTagValue2(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex);
	float GetXZRatio();

	//fields
	std::vector<byte>* dataVec;

	std::string fileName;
	texture3DDesc texDesc;
	int maxGradientMagnitude = 0;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader2;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavDerivatiesGist;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavHistTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvTexture3D;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvHistTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvSecondDerivatiesTexture;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				MRIDescDataCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				bDerivatiesGist;

	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				HistTexture;

	std::shared_ptr<DX::DeviceResources>				deviceResources;

	bool isTextureLoaded = false;
};

class DicomSet3D {
public:
	DicomSet3D(std::vector<std::vector<byte>*>* input) {
		this->slices = new std::vector<DicomSlice3D*>();
		for (int i = 0; i < input->size(); ++i) {
			this->slices->push_back(new DicomSlice3D(input->at(i)));
			this->slices->at(i)->ReadDesc(input->size());
		}
		if (slices->size() > 0) { 
			this->texDesc = slices->at(0)->GetTexDesc();
			this->texDesc.slices = slices->size();
		}
		
	}
	texture3DDesc GetTexDesc() { return this->texDesc; };
	void CreateID3D11ShaderResourceView(
		const std::shared_ptr<DX::DeviceResources>& deviceResources,
		ID3D11Texture3D** tex,
		ID3D11ShaderResourceView**,  float thresholdValue
	);
	float* GetTextureFloatData() {return this->textureFloatData; }



private:
	std::vector<DicomSlice3D*>* slices;
	texture3DDesc texDesc;
	int maxGradientMagnitude = 0;
	float* histoData;
	float* textureFloatData;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader>			computeShader2;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavDerivatiesGist;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	uavHistTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvTexture3D;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvHistTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	srvSecondDerivatiesTexture;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				MRIDescDataCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				bDerivatiesGist;

	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bFirstDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture3D>				bSecondDerivatiesTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				HistTexture;

	std::shared_ptr<DX::DeviceResources>				deviceResources;

	bool isTextureLoaded = false;
};