#pragma once

#include <ppltasks.h>	// Для create_task
#include <stdexcept>

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Установка точки останова на этой строке для перехвата ошибок API Win32.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Функция, осуществляющая асинхронное чтение из двоичного файла.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([] (StorageFile^ file) 
		{
			return FileIO::ReadBufferAsync(file);
		}).then([] (Streams::IBuffer^ fileBuffer) -> std::vector<byte> 
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Преобразует длину в аппаратно-независимых пикселях (DIP) в длину в физических пикселях.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Округление до ближайшего целого.
	}

#if defined(_DEBUG)
	// Проверка поддержки уровней пакета SDK.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // Нет необходимости создавать настоящее аппаратное устройство.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Проверка уровней пакета SDK.
			nullptr,                    // Подойдет любой функциональный уровень.
			0,
			D3D11_SDK_VERSION,          // Всегда устанавливать равным D3D11_SDK_VERSION для приложений Магазина Windows.
			nullptr,                    // Нет необходимости сохранять ссылку на устройство D3D.
			nullptr,                    // Нет необходимости знать функциональный уровень.
			nullptr                     // Нет необходимости сохранять ссылку на контекст устройства D3D.
			);

		return SUCCEEDED(hr);
	}
#endif

	inline void CreateTextureSRV(ID3D11Texture3D* inputTexture, 
		ID3D11ShaderResourceView** outputSRV,
		ID3D11Device* device) {

		CD3D11_TEXTURE3D_DESC inputTextureDesc;
		inputTexture->GetDesc(&inputTextureDesc);
		if (!(D3D11_BIND_SHADER_RESOURCE & inputTextureDesc.BindFlags)) throw std::invalid_argument("Texture doesnt have srv bind flag");
		ThrowIfFailed(
			device->CreateShaderResourceView(inputTexture, NULL, outputSRV)
		);		
	}

	inline void CreateTextureUAV(ID3D11Texture3D* inputTexture,
		ID3D11UnorderedAccessView** outputUAV,
		ID3D11Device* device) {
		CD3D11_TEXTURE3D_DESC inputTextureDesc;
		inputTexture->GetDesc(&inputTextureDesc);
		if (!(D3D11_BIND_UNORDERED_ACCESS & inputTextureDesc.BindFlags)) throw std::invalid_argument("Texture doesnt have uav bind flag");
		ThrowIfFailed(
			device->CreateUnorderedAccessView(inputTexture, NULL, outputUAV)
		);		
	}

	//FUNCTION FOR THE ONE TEST DATA. DONT USE
	inline void LoadTestDat(ID3D11Texture3D ** outTexture, ID3D11Device* device) {
		FILE* pFile;
		if (fopen_s(&pFile, "TEST.dat", "rb") == 0) {

			uint16_t xDim;
			uint16_t yDim;
			uint16_t zDim;

			fread_s(&xDim, 2, 2, 1, pFile);
			fread_s(&yDim, 2, 2, 1, pFile);
			fread_s(&zDim, 2, 2, 1, pFile);

			unsigned int channels = 1;
			unsigned int bytesPerChannel = sizeof(float);
			unsigned int textureNumValues = xDim * yDim * zDim * channels;

			float* pTextureData = new float[channels * xDim * yDim * zDim];

			//load data
			char readBuffer;
			float maxValue = 0;
			for (unsigned int i = 0; i < textureNumValues; ++i) {
				unsigned char fByte;
				unsigned char sByte;
				fread_s(&fByte, 1, 1, 1, pFile);
				fread_s(&sByte, 1, 1, 1, pFile);
				uint16_t value = (uint16_t)fByte + (((uint16_t)sByte << 4) >> 4)* pow(2, 8);
				pTextureData[i] = (float)value;
				if (maxValue < (float)value) maxValue = (float)value;
			}
			//create texture
			CD3D11_TEXTURE3D_DESC textureDesc;
			textureDesc.Width = xDim;
			textureDesc.Height = yDim;
			textureDesc.Depth = zDim;
			textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
			textureDesc.MipLevels = 1;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;


			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = pTextureData;
			initData.SysMemPitch = xDim * channels * bytesPerChannel;
			initData.SysMemSlicePitch = xDim * yDim * channels * bytesPerChannel;
			device->CreateTexture3D(&textureDesc, &initData, outTexture);

			delete[] pTextureData;
		}


	}



	inline void LoadTestTexture3D1(ID3D11Texture3D** outTexture, ID3D11Device* device) {

		uint16_t xDim = 128;
		uint16_t yDim = 128;
		uint16_t zDim = 128;

		unsigned int channels = 1;
		unsigned int bytesPerChannel = sizeof(float);
		unsigned int textureNumValues = xDim * yDim * zDim * channels;

		float* pTextureData = new float[channels * xDim * yDim * zDim];

		//load data
		char readBuffer;
		float maxValue = 0;
		for (unsigned int i = 0; i < xDim; ++i) {
			float value = (int)(i / 32);
			for (unsigned int j = 0; j < yDim; ++j) {
				for (unsigned int c = 0; c < zDim; ++c) {
					pTextureData[i + j * xDim + c * xDim * yDim] = value;
				}
			}
		}
		//create texture
		CD3D11_TEXTURE3D_DESC textureDesc;
		textureDesc.Width = xDim;
		textureDesc.Height = yDim;
		textureDesc.Depth = zDim;
		textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		textureDesc.MipLevels = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;


		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = pTextureData;
		initData.SysMemPitch = xDim * channels * bytesPerChannel;
		initData.SysMemSlicePitch = xDim * yDim * channels * bytesPerChannel;
		device->CreateTexture3D(&textureDesc, &initData, outTexture);

		delete[] pTextureData;


	}
	
}
