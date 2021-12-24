#pragma once
#include <stdexcept>
#include "..\Common\DirectXHelper.h"

using namespace Microsoft::WRL;

//Гистограмма хранит трехмерную текстуру, репрезентирующую плотность комбинации значений
class Histogram {
public:
	//create empty 3d texture and its uav and srv
	Histogram(uint16_t width, uint16_t height, uint16_t length, ID3D11Device* device);
	~Histogram(){}

	ID3D11ShaderResourceView* GetPSRV();
	ID3D11UnorderedAccessView* GetPUAV();
	ID3D11ShaderResourceView** GetPPSRV();
	ID3D11UnorderedAccessView** GetPPUAV();

private:
	ComPtr<ID3D11Texture3D> texture3D;
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11UnorderedAccessView> uav;
};