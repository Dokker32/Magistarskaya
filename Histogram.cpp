#include "pch.h"
#include "Histogram.h"


Histogram::Histogram(uint16_t width, uint16_t height, uint16_t length, ID3D11Device* device) {
	if (width < 1 || height < 1 || length < 1) throw std::invalid_argument("invalid histogram dimension");
	//create texture3d DESC
	CD3D11_TEXTURE3D_DESC t3dDesc;
	ZeroMemory(&t3dDesc, sizeof(t3dDesc));
	t3dDesc.Width = width;
	t3dDesc.Height = height;
	t3dDesc.Depth = length;
	t3dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	t3dDesc.Format = DXGI_FORMAT_R32_UINT;
	t3dDesc.MipLevels = 1;
	t3dDesc.Usage = D3D11_USAGE_DEFAULT;
	//create init data
	uint32_t* data = new uint32_t(width*height);
	for (int i = 0; i < (width * height); ++i) {
		data[i] = 0;
	}
	//create subresources data
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	//create resource and views
	DX::ThrowIfFailed(
		device->CreateTexture3D(&t3dDesc, &initData, &texture3D)
	);
	DX::ThrowIfFailed(
		device->CreateShaderResourceView(texture3D.Get(), NULL, &srv)
	);
	DX::ThrowIfFailed(
		device->CreateUnorderedAccessView(texture3D.Get(), NULL, &uav)
	);

	return;
}

ID3D11ShaderResourceView*  Histogram::GetPSRV() {
	return srv.Get();
}
ID3D11UnorderedAccessView* Histogram::GetPUAV() {
	return uav.Get();
}
ID3D11ShaderResourceView** Histogram::GetPPSRV() {
	return srv.GetAddressOf();
}
ID3D11UnorderedAccessView** Histogram::GetPPUAV() {
	return uav.GetAddressOf();
}

