#include "pch.h"
#include "dicom3D.h"
#include "vector"
#include "..\Common\DirectXHelper.h"
#include <d3dcompiler.h>



bool Dicom3D::CompareTag(unsigned char* tag, unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	if (tag[0] == firstHex && tag[1] == secondHex && tag[2] == thirdHex && tag[3] == fourthHex) return true;
	else return false;
}

Dicom3D::~Dicom3D() {
	delete[] outputData;
}


unsigned char Invert(unsigned char x)
{
	uint8_t base = pow(2, 8);

	unsigned char  res = 0;
	while (x != 0)
	{
		res += (x & 1) * (base >>= 1);
		x >>= 1;
	}

	return res;
}

void Dicom3D::FindTag(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	unsigned char rByte;
	fseek(this->pFile, 0, SEEK_SET);
	int count = 0;
	while (feof(this->pFile) == 0) {
		if (ferror(this->pFile) != 0) break;
		fread_s(&rByte, 1, 1, 1, this->pFile);

		if (count == 3) {
			if (rByte == fourthHex) ++count;
			else count = 0;
		}
		if (count == 2) {
			if (rByte == thirdHex) ++count;
			else count = 0;
		}
		if (count == 1) {
			if (rByte == secondHex) ++count;
			else count = 0;
		}
		if (count == 0) {
			if (rByte == firstHex) ++count;
		}

		if (count == 4) break;
	}
}

void Dicom3D::OpenFile() {
	if (fopen_s(&pFile, this->fileName.c_str(), "rb") == 0) isFileOpened = true;
}

void Dicom3D::CloseFile() {
	fclose(pFile);
	isFileOpened = false;
}

void Dicom3D::ShiftBy(long value) {
	if (isFileOpened) {
		fseek(pFile, value, SEEK_CUR);
	}
}

int Dicom3D::GetTagValue(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	if (isFileOpened) {
		this->FindTag(firstHex, secondHex, thirdHex, fourthHex);
		this->ShiftBy(2); //скипаем value representation
		this->ShiftBy(2); //скипаем length
		unsigned char rByte;
		fread_s(&rByte, 1, 1, 1, this->pFile);
		return (unsigned int)rByte;
	}
	return 0x00;
}

int Dicom3D::GetTagValue2(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	if (isFileOpened) {
		//value representation
		char desc[3];
		desc[2] = '\0';
		this->FindTag(firstHex, secondHex, thirdHex, fourthHex);
		fread_s(&desc, 3, 2, 1, this->pFile);
		string valueRepresentation = string(desc);
		//длина 
		unsigned char fByte;
		unsigned char sByte;
		fread_s(&fByte, 1, 1, 1, this->pFile);
		fread_s(&sByte, 1, 1, 1, this->pFile);
		uint16_t length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
		if (valueRepresentation=="US") {
			unsigned char* value = new unsigned char(length + 1);
			value[length] = '\0';
			for (int i = 0; i < length; ++i) {
				fread_s(value + i, 1, 1, 1, this->pFile);
			}
			int outputValue;
			ZeroMemory(&outputValue, sizeof(int));
			if (sizeof(typeid(outputValue)) < length) return 0x00;
			for (int i = 0; i < length; ++i) { //порядок обхода от 1го байта к ласт
				outputValue = outputValue | value[i] << i * 8;
			}
			return outputValue;
		}
		if (valueRepresentation=="IS") {
			char* charValue = new char(length + 1);
			charValue[length] = '\0';
			for (int i = 0; i < length; ++i) {
				fread_s(charValue + i, 1, 1, 1, this->pFile);
			}
			int outputValue = std::atoi(charValue);
			return outputValue;
		}
	}
	return 0x00;
}

float Dicom3D::GetXZRatio() {
	if (isFileOpened) {
		float xSize;
		float zSize;
		//value representation
		char desc[3];
		desc[2] = '\0';
		this->FindTag(0x28, 0x00, 0x30, 0x00);
		fread_s(&desc, 3, 2, 1, this->pFile);
		string valueRepresentation = string(desc);
		//длина 
		unsigned char fByte;
		unsigned char sByte;
		fread_s(&fByte, 1, 1, 1, this->pFile);
		fread_s(&sByte, 1, 1, 1, this->pFile);
		uint16_t length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
		if (valueRepresentation == "DS") {
			char* charValue = new char(length + 1);
			charValue[length] = '\0';
			for (int i = 0; i < length; ++i) {
				fread_s(charValue + i, 1, 1, 1, this->pFile);
				if (charValue[i] == '\\') {
					charValue[i] = '\0';
					break;
				}
			}
			xSize = std::atof(charValue+1);			
		}

		//zSize
		this->FindTag(0x18, 0x00, 0x50, 0x00);
		desc[2] = '\0';
		fread_s(&desc, 3, 2, 1, this->pFile);
		valueRepresentation = string(desc);
		//длина 
		fread_s(&fByte, 1, 1, 1, this->pFile);
		fread_s(&sByte, 1, 1, 1, this->pFile);
		length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
		if (valueRepresentation == "DS") {
			char* charValue = new char(length + 1);
			charValue[length] = '\0';
			for (int i = 0; i < length; ++i) {
				fread_s(charValue + i, 1, 1, 1, this->pFile);
			}
			zSize = std::atof(charValue);
		}

		return xSize/zSize;

	}
	return 0x00;

}

void Dicom3D::ReadDesc() {
	if (isFileOpened) {
		texDesc.columns = this->GetTagValue2(0x28, 0x00, 0x11, 0x00);
		texDesc.rows = this->GetTagValue2(0x28, 0x00, 0x10, 0x00);
		texDesc.slices = this->GetTagValue2(0x28, 0x00, 0x08, 0x00);
		texDesc.bitsAllocated = this->GetTagValue2(0x28, 0x00, 0x00, 0x01);
		texDesc.bitsActually = this->GetTagValue2(0x28, 0x00, 0x01, 0x01);
		texDesc.bitsHight = this->GetTagValue2(0x28, 0x00, 0x02, 0x01);
		texDesc.channels = this->GetTagValue2(0x28, 0x00, 0x02, 0x00);
		texDesc.zCompression = GetXZRatio()* texDesc.columns / texDesc.slices;
	}
}

texture3DDesc Dicom3D::GetTexDesc() {
	return texDesc;
}

void Dicom3D::CreateID3D11ShaderResourceView(
	const std::shared_ptr<DX::DeviceResources>& deviceResources,
	  ID3D11ShaderResourceView* * out, float** histoData, int& numColumns, float thresholdValue
) {
	if (isFileOpened) {
		this->deviceResources = deviceResources;
		uint16_t cWidth = texDesc.columns;
		uint16_t cHeight = texDesc.rows;
		uint16_t cDepth = texDesc.slices;

		D3D11_TEXTURE3D_DESC texureDesc;
		texureDesc.Width = cWidth;
		texureDesc.Height = cHeight;
		texureDesc.MipLevels = 1;
		texureDesc.Depth = cDepth;
		texureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		texureDesc.Usage = D3D11_USAGE_DEFAULT;
		texureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texureDesc.CPUAccessFlags = 0;
		texureDesc.MiscFlags = 0;

		outputData = new float[cWidth * cHeight * cDepth];
		ZeroMemory(outputData, cWidth * cHeight * cDepth * sizeof(float));
		int colorIndex = 0;

		this->FindTag(0xE0, 0x7F, 0x10, 0x00);
		this->ShiftBy(8);

		int maxValue = 0;
		for (uint16_t z = 0; z < cDepth; z++)
		{
			for (uint16_t y = 0; y < cHeight; y++)
			{
				for (uint16_t x = 0; (x < cWidth) && (feof(pFile) == 0); x++)
				{
					unsigned char fByte;
					unsigned char sByte;
					fread_s(&fByte, 1, 1, 1, this->pFile);
					fread_s(&sByte, 1, 1, 1, this->pFile);
					uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4)* pow(2, 8);
					if (value > thresholdValue) outputData[(0 + x + y * cWidth + z * cWidth * cHeight)] = (float)value;
					if (maxValue < value) maxValue = value;
				}
			}
		}
		if (maxValue>pow(2,10)) throw std::invalid_argument("too big max value in histo");
		this->maxGradientMagnitude = maxValue;
		numColumns = maxValue;
		*histoData = new float[numColumns + 1];
		ZeroMemory(*histoData, (numColumns + 1) * sizeof(float));
		//create histo data
		this->FindTag(0xE0, 0x7F, 0x10, 0x00);
		this->ShiftBy(8);

		for (uint16_t z = 0; z < cDepth; z++)
		{
			for (uint16_t y = 0; y < cHeight; y++)
			{
				for (uint16_t x = 0; (x < cWidth) && (feof(pFile) == 0); x++)
				{
					unsigned char fByte;
					unsigned char sByte;
					fread_s(&fByte, 1, 1, 1, this->pFile);
					fread_s(&sByte, 1, 1, 1, this->pFile);
					uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4)* pow(2, 8);
					if (value > thresholdValue) (*histoData)[(int)(value / (maxValue / (float)numColumns))] += 1;
				}
			}
		}
		(*histoData)[100];
		
		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = outputData;
		initData.SysMemPitch = cWidth * sizeof(float);
		initData.SysMemSlicePitch = cWidth * cHeight * sizeof(float);

		HRESULT hr;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tex = nullptr;
		hr = deviceResources->GetD3DDevice()->CreateTexture3D(&texureDesc, &initData, &tex);

		hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(tex.Get(), NULL, out);
		
		hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(tex.Get(), NULL, srvTexture3D.GetAddressOf());	

		isTextureLoaded = true;

		bool test = CalculateDerivatiesGist();

		delete[] outputData;

		return;
	}
}

bool Dicom3D::CalculateDerivatiesGist() {

	if (isTextureLoaded) {


		int width = texDesc.columns;
		int height = texDesc.rows;
		int length = texDesc.slices;
		HRESULT hr;

		//delete below
		Microsoft::WRL::ComPtr<ID3D11Texture3D> ttt;
		DX::LoadTestDat(&ttt, deviceResources->GetD3DDevice());
		//deviceResources->GetD3DDevice()->CreateShaderResourceView(ttt.Get(), NULL, srvTexture3D.ReleaseAndGetAddressOf());
		CD3D11_TEXTURE3D_DESC tttt;
		ttt.Get()->GetDesc(&tttt);
		width = tttt.Width;
		height = tttt.Height;
		length = tttt.Depth;

		
		//delete above

		//auto direct3DApplicationSource = ref new Direct3DApplicationSource();
		//Windows::ApplicationModel::Core::CoreApplication::Run(direct3DApplicationSource);
		

		//delete below!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//DX::LoadTestTexture3D1(&ttt, deviceResources->GetD3DDevice());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(ttt.Get(), NULL, srvTexture3D.ReleaseAndGetAddressOf());
		ttt.Get()->GetDesc(&tttt);
		width = tttt.Width;
		height = tttt.Height;
		length = tttt.Depth;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tttConv;		
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tttFDeriv;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tttSDeriv;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tttHist;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvConv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFDeriv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSDeriv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvHist;
		ApplyConvolution(ttt.Get(), tttConv.ReleaseAndGetAddressOf(),
			deviceResources->GetD3DDevice(),deviceResources->GetD3DDeviceContext(), 3, 1);

		CalculateHist3D(ttt.Get(), tttHist.ReleaseAndGetAddressOf(), 
			deviceResources->GetD3DDevice(), deviceResources->GetD3DDeviceContext(), 256, tttFDeriv.ReleaseAndGetAddressOf(), tttSDeriv.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttHist.Get(), NULL, srvHist.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttConv.Get(), NULL, srvConv.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttFDeriv.Get(), NULL, srvFDeriv.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttSDeriv.Get(), NULL, srvSDeriv.ReleaseAndGetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> tttHist2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvHist2D;
		CalculateHist2D(ttt.Get(), tttHist2D.ReleaseAndGetAddressOf(), deviceResources->GetD3DDevice(), 
			deviceResources->GetD3DDeviceContext(), 1024);
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttHist2D.Get(), NULL, srvHist2D.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDeviceContext()->CSSetShaderResources(6,1, srvHist2D.GetAddressOf());
		//deviceResources->GetD3DDeviceContext()->CSSetShaderResources(4, 1, srvHist.GetAddressOf());

		/*
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttOutput.Get(), NULL, tttOutputSRV.ReleaseAndGetAddressOf());

		deviceResources->GetD3DDeviceContext()->CSSetShaderResources(7,1, tttOutputSRV.GetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttOutput.Get(), NULL, srvTexture3D.ReleaseAndGetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Texture3D> tttOutput2;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tttOutputSRV2;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tttOutputSRV23;
		CalculateSecondDerivativeAlongGradient(tttOutput.Get(), tttOutput2.ReleaseAndGetAddressOf(),
			deviceResources->GetD3DDevice(), deviceResources->GetD3DDeviceContext());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttOutput2.Get(), NULL, tttOutputSRV23.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDevice()->CreateShaderResourceView(tttOutput2.Get(), NULL, tttOutputSRV2.ReleaseAndGetAddressOf());
		deviceResources->GetD3DDeviceContext()->CSSetShaderResources(6, 1, tttOutputSRV2.GetAddressOf());*/
		
		//delete above!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		

		D3D11_TEXTURE3D_DESC textureDesc = {};
		//ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Depth = length;
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		hr = deviceResources->GetD3DDevice()->CreateTexture3D(&textureDesc, 0, bFirstDerivatiesTexture.GetAddressOf());
		hr = deviceResources->GetD3DDevice()->CreateTexture3D(&textureDesc, 0, bSecondDerivatiesTexture.GetAddressOf());		

		DX::CreateTextureSRV(bFirstDerivatiesTexture.Get(), srvFirstDerivatiesTexture.GetAddressOf(), deviceResources->GetD3DDevice());
		//hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(bFirstDerivatiesTexture.Get(), NULL, srvFirstDerivatiesTexture.GetAddressOf());
		hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(bSecondDerivatiesTexture.Get(), NULL, srvSecondDerivatiesTexture.GetAddressOf());
		
		hr = deviceResources->GetD3DDevice()->CreateUnorderedAccessView(bFirstDerivatiesTexture.Get(), NULL, uavFirstDerivatiesTexture.GetAddressOf());
		hr = deviceResources->GetD3DDevice()->CreateUnorderedAccessView(bSecondDerivatiesTexture.Get(), NULL, uavSecondDerivatiesTexture.GetAddressOf());

		//describe mri
		struct MRI_DESC_DATA{
			int width;
			int height;
			int length;
			float padding;
		} MRIDescDataInst; 

		MRIDescDataInst.width = width;
		MRIDescDataInst.height = height;
		MRIDescDataInst.length = length;

		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.ByteWidth = sizeof(MRI_DESC_DATA);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &MRIDescDataInst;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		hr = deviceResources->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData,	&MRIDescDataCB);
		deviceResources->GetD3DDeviceContext()->CSSetConstantBuffers1(0, 1, MRIDescDataCB.GetAddressOf(), nullptr, nullptr);

		ID3D11SamplerState* SS;

		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.BorderColor[0] = 0.0f;
		sampDesc.BorderColor[1] = 0.0f;
		sampDesc.BorderColor[2] = 0.0f;
		sampDesc.BorderColor[3] = 0.0f;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, &SS);
		deviceResources->GetD3DDeviceContext()->CSSetSamplers(0, 1, &SS);

		ID3DBlob* fileData;

		D3DReadFileToBlob(L"ComputeShader.cso", &fileData);
		deviceResources->GetD3DDevice()->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader);
		fileData->Release();
		
		//calculate
		auto context = deviceResources->GetD3DDeviceContext();
		context->CSSetShader(computeShader.Get(), NULL, 0);
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		context->PSSetShaderResources(5, 1, nullSRV);
		context->PSSetShaderResources(6, 1, nullSRV);
		context->CSSetUnorderedAccessViews(0, 1, uavFirstDerivatiesTexture.GetAddressOf(), nullptr);
		context->CSSetUnorderedAccessViews(1, 1, uavSecondDerivatiesTexture.GetAddressOf(), nullptr);
		deviceResources->GetD3DDeviceContext()->CSSetShaderResources(0, 1, srvTexture3D.GetAddressOf());
		//be care here:
		int xThreadsInCS = 32;
		int yThreadsInCS = 32;
		int zThreadsInCS = 1;
		int xGropusOfThreads = width / xThreadsInCS;
		int yGropusOfThreads = height / yThreadsInCS;
		int zGropusOfThreads = length / zThreadsInCS;

		context->Dispatch(xGropusOfThreads, yGropusOfThreads, zGropusOfThreads);

		ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
		context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
		//context->CSSetUnorderedAccessViews(1, 1, nullUAV, nullptr);
		//context->PSSetShaderResources(5, 1, srvFirstDerivatiesTexture.GetAddressOf());
		//context->PSSetShaderResources(6, 1, srvSecondDerivatiesTexture.GetAddressOf());
		

		//create hist
		D3D11_TEXTURE2D_DESC textureDesc2D = {0};
		textureDesc2D.Width = 256;
		textureDesc2D.Height = 256;
		textureDesc2D.MipLevels = 1;
		textureDesc2D.Format = DXGI_FORMAT_R32_UINT;
		textureDesc2D.Usage = D3D11_USAGE_DEFAULT;
		textureDesc2D.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		textureDesc2D.CPUAccessFlags = 0;
		textureDesc2D.MiscFlags = 0;
		textureDesc2D.ArraySize = 1;
		textureDesc2D.SampleDesc.Count = 1;

		const uint16_t bytesPerChannel = 4;
		const uint16_t channels = 1;
		if (sizeof(unsigned int) != bytesPerChannel ) return false;
		unsigned int* tex3d = new unsigned int[textureDesc2D.Width * textureDesc2D.Height * channels];
		for (int i = 0; i < (textureDesc2D.Width * textureDesc2D.Height); ++i) {
			tex3d[i] = 0;
		}

		D3D11_SUBRESOURCE_DATA initData4 = { 0 };
		initData4.pSysMem = &tex3d;
		initData4.SysMemPitch = textureDesc2D.Width * bytesPerChannel * channels; //in bytes.
		initData4.SysMemSlicePitch = 0;

		hr = deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc2D, nullptr, HistTexture.GetAddressOf());
		hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(HistTexture.Get(), NULL, 
			srvHistTexture.GetAddressOf());
		hr = deviceResources->GetD3DDevice()->CreateUnorderedAccessView(HistTexture.Get(), NULL,
			uavHistTexture.GetAddressOf());

		//describe data
		struct DERIV_DESC_DATA {
			float maxValueFDM;
			float maxValueSDM;
			float minValueSDM;
			float maxScalar;
		} DerivDescDataInst;

		//DerivDescDataInst.maxValueFDM = maxGradientMagnitude;
		DerivDescDataInst.maxValueFDM = 4; //delete
		DerivDescDataInst.maxValueFDM = 0.3; //delete
		DerivDescDataInst.maxScalar = 4;//delete
		DerivDescDataInst.minValueSDM = -0.3;
		//DerivDescDataInst.maxValueFDM = 4095; //delete 
		DerivDescDataInst.maxValueSDM = 0;

		D3D11_BUFFER_DESC cb2Desc;
		ZeroMemory(&cb2Desc, sizeof(cb2Desc));
		cb2Desc.ByteWidth = sizeof(DerivDescDataInst);
		cb2Desc.Usage = D3D11_USAGE_DEFAULT;
		cb2Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb2Desc.CPUAccessFlags = 0;
		cb2Desc.MiscFlags = 0;
		cb2Desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData2;
		InitData2.pSysMem = &DerivDescDataInst;
		InitData2.SysMemPitch = 0;
		InitData2.SysMemSlicePitch = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> DerivDescDataCB;
		hr = deviceResources->GetD3DDevice()->CreateBuffer(&cb2Desc, &InitData2, &DerivDescDataCB);
		deviceResources->GetD3DDeviceContext()->CSSetConstantBuffers1(1, 1, DerivDescDataCB.GetAddressOf(), nullptr, nullptr);

		ID3DBlob* fileData2;
		D3DReadFileToBlob(L"CSHist.cso", &fileData2);
		hr = deviceResources->GetD3DDevice()->CreateComputeShader(fileData2->GetBufferPointer(), fileData2->GetBufferSize(), NULL, &computeShader2);

		context->CSSetShaderResources(1, 1, srvSDeriv.GetAddressOf());
		context->CSSetUnorderedAccessViews(0,1, uavHistTexture.GetAddressOf(), nullptr);

		context->CSSetShader(computeShader2.Get(), NULL, 0);
		context->Dispatch(xGropusOfThreads, yGropusOfThreads, zGropusOfThreads);

		context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);


		context->PSSetShaderResources(0, 1, srvHist.GetAddressOf());

		

		return true;
	}
	return false;
}

void Dicom3D::GetRawData(float** outputData, int** outputWidth, int** outputHeight, int** outputSlices) {
	this->OpenFile();
	this->ReadDesc();

	this->FindTag(0xE0, 0x7F, 0x10, 0x00);
	this->ShiftBy(8);

	if (*outputData != nullptr) delete[] outputData;
	*outputData = new float[texDesc.columns * texDesc.rows * texDesc.slices];
	ZeroMemory(*outputData, texDesc.columns * texDesc.rows * texDesc.slices * sizeof(float));

	if (*outputWidth != nullptr) delete *outputWidth;
	*outputWidth = new int (texDesc.columns);
	if (*outputHeight != nullptr) delete *outputHeight;
	*outputHeight = new int (texDesc.rows);
	if (*outputSlices != nullptr) delete *outputSlices;
	*outputSlices = new int (texDesc.slices);

	for (uint16_t z = 0; z < texDesc.slices; z++)
	{
		for (uint16_t y = 0; y < texDesc.rows; y++)
		{
			for (uint16_t x = 0; (x < texDesc.columns) && (feof(pFile) == 0); x++)
			{
				unsigned char fByte;
				unsigned char sByte;
				fread_s(&fByte, 1, 1, 1, this->pFile);
				fread_s(&sByte, 1, 1, 1, this->pFile);
				uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4) * pow(2, 8);
				if (value > 50.0) (*outputData)[(0 + x + y * texDesc.columns + z * texDesc.columns * texDesc.rows)] = (float)value;
				if (this->maxValue < value) this->maxValue = value;
			}
		}
	}

	this->CloseFile();
}

//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------


bool DicomSlice3D::CompareTag(unsigned char* tag, unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	if (tag[0] == firstHex && tag[1] == secondHex && tag[2] == thirdHex && tag[3] == fourthHex) return true;
	else return false;
}


void DicomSlice3D::FindTag(int* pos, unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {
	unsigned char rByte;	
	int count = 0;
	for (auto it = dataVec->begin(); it != dataVec->end(); ++it) {
		if (count == 3) {
			if ((*it) == fourthHex) ++count;
			else count = 0;
		}
		if (count == 2) {
			if ((*it) == thirdHex) ++count;
			else count = 0;
		}
		if (count == 1) {
			if ((*it) == secondHex) ++count;
			else count = 0;
		}
		if (count == 0) {
			if ((*it) == firstHex) ++count;
		}

		if (count == 4) { 
			*pos = it - dataVec->begin()+1;
			break; 
		}
	}
}


int DicomSlice3D::GetTagValue2(unsigned char firstHex, unsigned char secondHex, unsigned char thirdHex, unsigned char fourthHex) {

	//value representation
	char desc[3];
	desc[2] = '\0';
	int pos = 0;
	this->FindTag(&pos, firstHex, secondHex, thirdHex, fourthHex);
	desc[0] = dataVec->at(pos++);
	desc[1] = dataVec->at(pos++);
	string valueRepresentation = string(desc);
	//длина 
	unsigned char fByte;
	unsigned char sByte;
	fByte = dataVec->at(pos++);
	sByte = dataVec->at(pos++);
	uint16_t length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
	if (valueRepresentation == "US") {
		unsigned char* value = new unsigned char(length + 1);
		value[length] = '\0';
		for (int i = 0; i < length; ++i) {
			*(value + i) = dataVec->at(pos++);
		}
		int outputValue;
		ZeroMemory(&outputValue, sizeof(int));
		if (sizeof(typeid(outputValue)) < length) return 0x00;
		for (int i = 0; i < length; ++i) { //порядок обхода от 1го байта к ласт
			outputValue = outputValue | value[i] << i * 8;
		}
		return outputValue;
	}
	if (valueRepresentation == "IS") {
		char* charValue = new char(length + 1);
		charValue[length] = '\0';
		for (int i = 0; i < length; ++i) {
			*(charValue + i) = dataVec->at(pos++);
		}
		int outputValue = std::atoi(charValue);
		return outputValue;
	}
	
	return 0x00;
}

float DicomSlice3D::GetXZRatio() {

	float xSize;
	float zSize;
	//value representation
	char desc[3];
	desc[2] = '\0';
	int pos = 0;
	this->FindTag(&pos, 0x28, 0x00, 0x30, 0x00);
	desc[0] = dataVec->at(pos++);
	desc[1] = dataVec->at(pos++);
	string valueRepresentation = string(desc);
	//длина 
	unsigned char fByte;
	unsigned char sByte;
	fByte = dataVec->at(pos++);
	sByte = dataVec->at(pos++);
	uint16_t length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
	if (valueRepresentation == "DS") {
		char* charValue = new char(length + 1);
		charValue[length] = '\0';
		for (int i = 0; i < length; ++i) {
			*(charValue + i) = dataVec->at(pos++);
			if (charValue[i] == '\\') {
				charValue[i] = '\0';
				break;
			}
		}
		xSize = std::atof(charValue + 1);
	}

	//zSize
	this->FindTag(&pos, 0x28, 0x00, 0x30, 0x00);
	desc[2] = '\0';
	desc[0] = dataVec->at(pos++);
	desc[1] = dataVec->at(pos++);
	valueRepresentation = string(desc);
	//длина 
	fByte = dataVec->at(pos++);
	sByte = dataVec->at(pos++);
	length = (uint16_t)fByte | (((uint16_t)sByte << 4) >> 4) << 8;
	if (valueRepresentation == "DS") {
		char* charValue = new char(length + 1);
		charValue[length] = '\0';
		for (int i = 0; i < length; ++i) {
			*(charValue + i) = dataVec->at(pos++);
		}
		zSize = std::atof(charValue);
	}

	return xSize / zSize;

	
	return 0x00;

}

void DicomSlice3D::ReadDesc(int numFiles) {

	texDesc.columns = this->GetTagValue2(0x28, 0x00, 0x11, 0x00);
	texDesc.rows = this->GetTagValue2(0x28, 0x00, 0x10, 0x00);
	texDesc.slices = this->GetTagValue2(0x28, 0x00, 0x08, 0x00);
	texDesc.bitsAllocated = this->GetTagValue2(0x28, 0x00, 0x00, 0x01);
	texDesc.bitsActually = this->GetTagValue2(0x28, 0x00, 0x01, 0x01);
	texDesc.bitsHight = this->GetTagValue2(0x28, 0x00, 0x02, 0x01);
	texDesc.channels = this->GetTagValue2(0x28, 0x00, 0x02, 0x00);
	if (texDesc.slices < 1) texDesc.zCompression = GetXZRatio()* texDesc.columns / numFiles;
	else texDesc.zCompression = GetXZRatio() * texDesc.columns / texDesc.slices;
	
}

texture3DDesc DicomSlice3D::GetTexDesc() {
	return texDesc;
}

void DicomSlice3D::GetData(std::vector<byte>** outputVector, int* position) {
	*outputVector = this->dataVec;
	this->FindTag(position, 0xE0, 0x7F, 0x10, 0x00);
	*position += 8;
	return;	
}

//В текущем дикоме какого то фига данные после второго такого тэга я хз
void DicomSlice3D::GetDataSecond(std::vector<byte>** outputVector, int* position) {
	*outputVector = this->dataVec;
	this->FindTag(position, 0xE0, 0x7F, 0x10, 0x00);

	unsigned char firstHex = 0xE0;
	unsigned char secondHex = 0x7F;
	unsigned char thirdHex = 0x10;
	unsigned char fourthHex = 0x00;
	
	unsigned char rByte;
	int count = 0;
	for (auto it = dataVec->begin(); it != dataVec->end(); ++it) {
		if (count == 3) {
			if ((*it) == fourthHex) ++count;
			else count = 0;
		}
		if (count == 2) {
			if ((*it) == thirdHex) ++count;
			else count = 0;
		}
		if (count == 1) {
			if ((*it) == secondHex) ++count;
			else count = 0;
		}
		if (count == 0) {
			if ((*it) == firstHex) ++count;
		}

		if (count == 4) {
			*position = it - dataVec->begin() + 1;
			break;
		}
	}

	 count = 0;
	for (auto it = dataVec->begin() + *position; it != dataVec->end(); ++it) {
		if (count == 3) {
			if ((*it) == fourthHex) ++count;
			else count = 0;
		}
		if (count == 2) {
			if ((*it) == thirdHex) ++count;
			else count = 0;
		}
		if (count == 1) {
			if ((*it) == secondHex) ++count;
			else count = 0;
		}
		if (count == 0) {
			if ((*it) == firstHex) ++count;
		}

		if (count == 4) {
			*position = it - dataVec->begin()+1;
			break;
		}
	}
	

	*position += 8;
	return;
}


void DicomSlice3D::GetDataSize(int* dataSize) {
	int position;
	this->FindTag(&position, 0xE0, 0x7F, 0x10, 0x00);
	*dataSize = (this->dataVec->size()-(position+8))/2;
	return;
}

void DicomSet3D::CreateID3D11ShaderResourceView(
	const std::shared_ptr<DX::DeviceResources>& deviceResources,
	ID3D11Texture3D** tex,
	ID3D11ShaderResourceView** out, float thresholdValue
) {
	int maxValue = 0;

	this->deviceResources = deviceResources;
	uint16_t cWidth = texDesc.columns;
	uint16_t cHeight = texDesc.rows;
	uint16_t cDepth = this->slices->size();

	//test
	uint16_t rWidth = 256;
	uint16_t rHeight = 256;
	//uint16_t rDepth = 256;

	D3D11_TEXTURE3D_DESC textureDesc;
	textureDesc.Width = rWidth;
	textureDesc.Height = rHeight;
	textureDesc.MipLevels = 1;
	textureDesc.Depth = cDepth;
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//copy data into texture
	int pPos = 0;
	textureFloatData = new float[rWidth * rHeight * cDepth];
	ZeroMemory(textureFloatData, rWidth * rHeight * cDepth * sizeof(float));
	for (uint16_t z = 0; z < cDepth; ++z) {
		int dataPos = 0;
		std::vector<byte>* tmpVec;
		this->slices->at(z)->GetDataSecond(&tmpVec, &dataPos);
		unsigned char fByte = 0;
		unsigned char sByte = 0;
		int pointerVecPos = 0;

		for (uint16_t y = 0; y < cHeight; y++)
		{
			for (uint16_t x = 0; x < cWidth; x++)
			{
				fByte = tmpVec->at(dataPos + (x + y * cWidth) * 2);
				sByte = tmpVec->at(dataPos + (x + y * cWidth) * 2 + 1);
				uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4) * pow(2, 8);
				textureFloatData[x + y * rWidth + z * rWidth * rHeight] = (float)value;
				if (maxValue < (float)value) maxValue = (float)value;
			}

			/*
			for (auto it = tmpVec->begin(); it != tmpVec->end()-2; it+=2) {
				fByte = *(it);
				sByte = *(it+1);
				uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4) * pow(2, 8);
				//if (value > thresholdValue)
				*(textureFloatData + pPos) = (float)value;
				if (maxValue < value) maxValue = value;
				//pPos += 2;
			}*/
		}
		//std::copy(tmpVec->begin() + dataPos, tmpVec->end(), textureFloatData + pPos);
		//int tmpPos = 0;
		//this->slices->at(z)->GetDataSize(&tmpPos);
		//pPos += tmpPos;
	}
	
	if (maxValue > pow(2, 20)) throw std::invalid_argument("too big max value in histo");
	this->maxGradientMagnitude = maxValue;
	int numColumns = maxValue;
	histoData = new float[numColumns + 1];
	ZeroMemory(histoData, (numColumns + 1) * sizeof(float));
	/*
	//create histo 1D data
	for (uint16_t z = 0; z < 1; z++)
	{
		int dataPos = 0;
		std::vector<byte>* tmpVec;
		this->slices->at(z)->GetData(&tmpVec, &dataPos);

		unsigned char fByte = 0;
		unsigned char sByte = 0;
		int pointerVecPos = 0;

		for (uint16_t y = 0; y < cHeight; y++)
		{
			for (uint16_t x = 0; x < cWidth; x++)
			{
				fByte = tmpVec->at((dataPos + x + y * cWidth) * 2);
				sByte = tmpVec->at((dataPos + x + y * cWidth) * 2 + 1);
				uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4) * pow(2, 8);
				if (value > thresholdValue) (histoData)[(int)(value / (maxValue / (float)numColumns))] += 1;
			}
		}
	}*/
	
	//ZeroMemory(textureFloatData, cWidth * cHeight * sizeof(float));
	//for (int i = 0; i < 256 * 256 * cDepth; ++i)
		//this->textureFloatData[i] = i;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = textureFloatData;
	initData.SysMemPitch = rWidth * sizeof(float);
	initData.SysMemSlicePitch = rWidth * rHeight * sizeof(float);

	HRESULT hr;
	//hr = deviceResources->GetD3DDevice()->CreateTexture3D(&textureDesc, &initData, tex);

	//hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(*tex, NULL, out);
	isTextureLoaded = true;

	return;
	
}
