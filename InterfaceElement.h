#pragma once
#include "DirectXHelpers.h"
#include "Common/DeviceResources.h"
#include "helper.h"
#include "..\Common\DirectXHelper.h"

class InterfaceElement {
public:

	bool Initialize(uint16_t pixelHight, uint16_t pixelWidth,
		const std::shared_ptr<DX::DeviceResources>& deviceResources, float maxValue, float* histoData, int );
	// Рендерит элемент интерфейса
	bool Render();
	//set LH and RD position
	bool SetPosition(DirectX::XMFLOAT2, DirectX::XMFLOAT2);
	bool createBackground(float maxValue, float* histoData, int);
	

private:
	bool CreateAxes(float* tex3d, int);
	bool LoadShaders(const std::wstring&, const std::wstring&);
	bool CreateHisto(float* tex3d, float maxValue, float* histoData, int);


	DirectX::XMFLOAT2 LHPosition; //Left-High position in screen space coordinate
	DirectX::XMFLOAT2 RDPosition; //Right-Dawn position in screen space coordinate
	int pixelHight;
	int pixelWidth;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> background;
	std::shared_ptr<float> tex3d;
	float maxValue;
	IESquad sq;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		squadVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> squadIndexBuffer;
	uint16_t squadIndexCount;	
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	std::shared_ptr<DX::DeviceResources>		deviceResources;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	bool loadingComplete = false;

	int XAxesPixelOffset;
	int YAxesPixelOffset;
};