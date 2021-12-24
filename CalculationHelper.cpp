#include "pch.h"
#include "CalculationHelper.h"

using namespace Microsoft::WRL;


void CalculateGradientMagnitude6(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context) {

	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSGradientMagnitude6.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;
	
	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader)+1, (textureLength / zThreadsInShader)+1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	return;



}

void CalculateGradientMagnitude26(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context) {

	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSGradientMagnitude26.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;

	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader) + 1, (textureLength / zThreadsInShader) + 1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
	return;
}


void CalculateGradient6Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context)
{
	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSGradient6.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;

	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader) + 1, (textureLength / zThreadsInShader) + 1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	return;

}

void CalculateGradient26Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context)
{
	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSGradient26.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;

	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader) + 1, (textureLength / zThreadsInShader) + 1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	return;
}

void CalculateNodeGradient8Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context)
{
	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	outputTextureDesc.Width;
	outputTextureDesc.Height;
	outputTextureDesc.Depth;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSNodeGradient8.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;

	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader) + 1, (textureLength / zThreadsInShader) + 1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	return;
}

void CalculateNodeScalar8Points(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context)
{
	CD3D11_TEXTURE3D_DESC inputTextureDesc;
	CD3D11_TEXTURE3D_DESC outputTextureDesc;
	inputTexture->GetDesc(&inputTextureDesc);

	int textureWidth = inputTextureDesc.Width;
	int textureHeight = inputTextureDesc.Height;
	int textureLength = inputTextureDesc.Depth;

	outputTextureDesc = inputTextureDesc;
	outputTextureDesc.MipLevels = 1;
	outputTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	outputTextureDesc.CPUAccessFlags = 0;
	outputTextureDesc.MiscFlags = 0;
	outputTextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	outputTextureDesc.Width;
	outputTextureDesc.Height;
	outputTextureDesc.Depth;

	DX::ThrowIfFailed(device->CreateTexture3D(&outputTextureDesc, NULL, outputTexture));

	//load shader
	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSNodeScalar8.cso", &fileData);
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
	DX::ThrowIfFailed(
		device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &computeShader)
	);
	fileData->Release();

	//get srv from input texture
	ComPtr<ID3D11ShaderResourceView> srv;
	DX::CreateTextureSRV(inputTexture, srv.GetAddressOf(), device);
	//get uav from output texture
	ComPtr<ID3D11UnorderedAccessView> uav;
	DX::CreateTextureUAV(*outputTexture, uav.GetAddressOf(), device);

	context->CSSetShader(computeShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, srv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), nullptr);

	//DANGER!!!!!!
	int xThreadsInShader = 32;
	int yThreadsInShader = 32;
	int zThreadsInShader = 1;

	context->Dispatch((textureWidth / xThreadsInShader) + 1, (textureHeight / yThreadsInShader) + 1, (textureLength / zThreadsInShader) + 1);

	//release resources
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	return;
}


void ApplyConvolution(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, 
	ID3D11DeviceContext* context, uint32_t kernelRadius, float sigma) {
	CD3D11_TEXTURE3D_DESC inputTexDesc;
	inputTexture->GetDesc(&inputTexDesc);
	inputTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

	device->CreateTexture3D(&inputTexDesc, NULL, outputTexture);

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;

	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSConvolution.cso", &fileData);
	DX::ThrowIfFailed(device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, ComputeShader.ReleaseAndGetAddressOf()));
	fileData->Release();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> outputUAV;
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture, NULL, inputSRV.ReleaseAndGetAddressOf()));	
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(*outputTexture, NULL, outputUAV.ReleaseAndGetAddressOf()));	

	//create cb
	Microsoft::WRL::ComPtr<ID3D11Buffer> cbBuffer;
	struct scbDimDesc {
		uint32_t width;
		uint32_t height;
		uint32_t length;
		uint32_t kernelRadius;
	} cbDimDesc;

	cbDimDesc.width = inputTexDesc.Width;
	cbDimDesc.height = inputTexDesc.Height;
	cbDimDesc.length = inputTexDesc.Depth;
	cbDimDesc.kernelRadius = (uint32_t)kernelRadius;

	uint32_t width = inputTexDesc.Width;
	uint32_t height = inputTexDesc.Height;
	uint32_t length = inputTexDesc.Depth;

	CD3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(scbDimDesc);
	cbDesc.MiscFlags = 0;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &cbDimDesc;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, &initData, cbBuffer.ReleaseAndGetAddressOf()));
	
	//create sampler
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	CD3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, sampler.ReleaseAndGetAddressOf()));
	//create kernel
	Microsoft::WRL::ComPtr<ID3D11Texture3D> kernelTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> kernelTextureSRV;
	createGaussKernel(kernelRadius, sigma, kernelTexture.ReleaseAndGetAddressOf(), device);

	DX::ThrowIfFailed(device->CreateShaderResourceView(kernelTexture.Get(), NULL, kernelTextureSRV.ReleaseAndGetAddressOf()));
	//dispatch
	context->CSSetShader(ComputeShader.Get(), NULL, 0);
	context->CSSetConstantBuffers(0,1,cbBuffer.GetAddressOf());
	context->CSSetShaderResources(0,1,inputSRV.GetAddressOf());
	context->CSSetShaderResources(7,1,kernelTextureSRV.GetAddressOf());
	context->CSSetSamplers(0,1, sampler.GetAddressOf());
	context->CSSetUnorderedAccessViews(0,1, outputUAV.GetAddressOf(), nullptr);

	int xTheads = (width+31)/32;
	int yTheads = (height+31)/32;
	int zTheads = length/1;

	context->Dispatch(xTheads, yTheads, zTheads);

	//release resources
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetShaderResources(1, 1, nullSRV);

	return;

}

void createGaussKernel(uint16_t radius, float sigma, ID3D11Texture3D** outputTexture, ID3D11Device* device) {
	if (radius<1) throw std::invalid_argument("radius less than one");
	uint16_t size = radius * 2 - 1;
	float constA = 1 / (2*3.14f*sigma*sigma)/sqrt(2 * 3.14f * sigma * sigma);

	CD3D11_TEXTURE3D_DESC textureDesc;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.Height = size;
	textureDesc.Width = size;
	textureDesc.Depth = size;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.MiscFlags = 0;

	float* kernel = new float[size * size * size];

	float div=0;

	for (uint16_t i = 0; i < size; ++i) {
		for (uint16_t j = 0; j < size; ++j) {
			for (uint16_t c = 0; c < size; ++c) {
				uint16_t deltaX = abs(radius - (j + 1));
				uint16_t deltaY = abs(radius - (i + 1));
				uint16_t deltaZ = abs(radius - (c + 1));
				kernel[j + i * size + c*size*size] = constA * pow(2.718f, -(deltaX * deltaX + deltaY*deltaY + deltaZ*deltaZ) / (2 * sigma * sigma));
				div += kernel[j + i * size + c * size * size];
			}
		}
	}

	for (uint16_t i = 0; i < size; ++i) {
		for (uint16_t j = 0; j < size; ++j) {
			for (uint16_t c = 0; c < size; ++c) {
				kernel[j + i * size + c * size * size] /= div;
			}
		}
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = kernel;
	initData.SysMemPitch = size*sizeof(float);
	initData.SysMemSlicePitch = size*size * sizeof(float);

	device->CreateTexture3D(&textureDesc, &initData, outputTexture);
	delete[] kernel;
	return;
}

void CalculateSecondDerivativeAlongGradient(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture,
	ID3D11Device* device, ID3D11DeviceContext* context) {

	CD3D11_TEXTURE3D_DESC inputTexDesc;
	inputTexture->GetDesc(&inputTexDesc);
	inputTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;


	device->CreateTexture3D(&inputTexDesc, NULL, outputTexture);

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;

	ID3DBlob* fileData;
	D3DReadFileToBlob(L"CSSecondGradientMagnitude.cso", &fileData);
	DX::ThrowIfFailed(device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, ComputeShader.ReleaseAndGetAddressOf()));
	fileData->Release();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> inputSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> outputUAV;
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture, NULL, inputSRV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(*outputTexture, NULL, outputUAV.ReleaseAndGetAddressOf()));

	//create cb
	Microsoft::WRL::ComPtr<ID3D11Buffer> cbBuffer;
	struct scbDimDesc {
		uint32_t width;
		uint32_t height;
		uint32_t length;
		uint32_t padding;
	} cbDimDesc;

	cbDimDesc.width = inputTexDesc.Width;
	cbDimDesc.height = inputTexDesc.Height;
	cbDimDesc.length = inputTexDesc.Depth;

	uint32_t width = inputTexDesc.Width;
	uint32_t height = inputTexDesc.Height;
	uint32_t length = inputTexDesc.Depth;

	CD3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(scbDimDesc);
	cbDesc.MiscFlags = 0;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &cbDimDesc;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, &initData, cbBuffer.ReleaseAndGetAddressOf()));

	//create sampler
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	CD3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, sampler.ReleaseAndGetAddressOf()));

	//Dispatch
	context->CSSetShader(ComputeShader.Get(), NULL, 0);
	context->CSSetConstantBuffers(0, 1, cbBuffer.GetAddressOf());
	context->CSSetShaderResources(0, 1, inputSRV.GetAddressOf());
	context->CSSetSamplers(0, 1, sampler.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, outputUAV.GetAddressOf(), nullptr);

	int xTheads = width / 32;
	int yTheads = height / 32;
	int zTheads = length / 1;

	context->Dispatch(xTheads, yTheads, zTheads);

	//release resources
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetShaderResources(1, 1, nullSRV);

	return;

}

void CalculateHist3D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2, ID3D11Texture3D* inputTexture3,
	ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize) {

	ComPtr<ID3D11UnorderedAccessView> uavOutput;
	ComPtr<ID3D11ComputeShader> computeShader;
	ComPtr<ID3D11ShaderResourceView> srvInput1;
	ComPtr<ID3D11ShaderResourceView> srvInput2;
	ComPtr<ID3D11ShaderResourceView> srvInput3;
	ComPtr<ID3D11Buffer> threadsParameters;
	ComPtr<ID3D11Buffer> textureParameters;

	//check resolution of input textures
	CD3D11_TEXTURE3D_DESC inputTexture1Desc;
	inputTexture1->GetDesc(&inputTexture1Desc);
	CD3D11_TEXTURE3D_DESC inputTexture2Desc;
	inputTexture2->GetDesc(&inputTexture2Desc);
	CD3D11_TEXTURE3D_DESC inputTexture3Desc;
	inputTexture3->GetDesc(&inputTexture3Desc);
	if (inputTexture1Desc.Width != inputTexture2Desc.Width || inputTexture2Desc.Width != inputTexture3Desc.Width) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Height != inputTexture2Desc.Height || inputTexture2Desc.Height != inputTexture3Desc.Height) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Depth != inputTexture2Desc.Depth || inputTexture2Desc.Depth != inputTexture3Desc.Depth) std::invalid_argument("different resolution of input textures");

	CD3D11_TEXTURE3D_DESC outputDesc;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.Width = outputSize;
	outputDesc.Height = outputSize;
	outputDesc.Depth = outputSize;
	outputDesc.Format = DXGI_FORMAT_R32_UINT;
	outputDesc.MipLevels = 1;
	outputDesc.MiscFlags = 0;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;

	uint32_t* data = new uint32_t[pow(outputSize,3)];
	
	ZeroMemory(data, sizeof(uint32_t)* pow(outputSize, 3));

	int channels = 1;
	int bytesPerChannel = 4;
	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = data;
	subData.SysMemPitch = outputSize* bytesPerChannel;
	subData.SysMemSlicePitch = pow(outputSize, 2) * bytesPerChannel;
	
	//create View
	DX::ThrowIfFailed(device->CreateTexture3D(&outputDesc, &subData, outputTexture));
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(*outputTexture, nullptr, uavOutput.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture1, nullptr, srvInput1.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture2, nullptr, srvInput2.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture3, nullptr, srvInput3.ReleaseAndGetAddressOf()));
	
	//load shader
	ID3D10Blob* fileData;
	D3DReadFileToBlob(L"CSHist3D.cso", &fileData);
	DX::ThrowIfFailed(device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), 
		NULL, computeShader.ReleaseAndGetAddressOf()));
	
	//find min max values
	float* minValueInputTex1 = new float();
	float* maxValueInputTex1 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture1, device, context, minValueInputTex1, maxValueInputTex1);

	float* minValueInputTex2 = new float();
	float* maxValueInputTex2 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture2, device, context, minValueInputTex2, maxValueInputTex2);

	float* minValueInputTex3 = new float();
	float* maxValueInputTex3 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture3, device, context, minValueInputTex3, maxValueInputTex3);

	//constant buffers
	struct sThreadsParameters {
		int threadWidth;
		int threadHeight;
		int threadLength;
		float padding;
	} bThreadsParameters;
	
	struct sTextureParameters
	{
		float minScalarValue;
		float maxScalarValue;
		float minFirstDerivValue;
		float maxFirstDerivValue;
		float minSecondDerivValue;
		float maxSecondDerivValue;
		float outputTextureSize;
		float padding;
	} bTextureParameters;

	int threadWidthInShader = 32;
	int threadHeightInShader = 32;
	int threadLengthInShader = 1;
	bThreadsParameters.threadWidth = inputTexture1Desc.Width;
	bThreadsParameters.threadHeight = inputTexture1Desc.Height;
	bThreadsParameters.threadLength = inputTexture1Desc.Depth;

	bTextureParameters.minScalarValue = *minValueInputTex1;
	bTextureParameters.maxScalarValue = *maxValueInputTex1;
	bTextureParameters.minFirstDerivValue = *minValueInputTex2;
	bTextureParameters.maxFirstDerivValue = *maxValueInputTex2;
	bTextureParameters.minSecondDerivValue = *minValueInputTex3;
	bTextureParameters.maxSecondDerivValue = *maxValueInputTex3;
	bTextureParameters.outputTextureSize = outputSize;

	CD3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(bThreadsParameters);
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem = &bThreadsParameters;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, threadsParameters.ReleaseAndGetAddressOf());

	bufferDesc.ByteWidth = sizeof(bTextureParameters);

	cbInitData.pSysMem = &bTextureParameters;
	cbInitData.SysMemPitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, textureParameters.ReleaseAndGetAddressOf());
	
	//Set resoucrs & dispatch
	context->CSSetConstantBuffers(0, 1, threadsParameters.GetAddressOf());
	context->CSSetConstantBuffers(1, 1, textureParameters.GetAddressOf());
	context->CSSetShaderResources(0, 1, srvInput1.GetAddressOf());
	context->CSSetShaderResources(1, 1, srvInput2.GetAddressOf());
	context->CSSetShaderResources(2, 1, srvInput3.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uavOutput.GetAddressOf(), nullptr);
	context->CSSetShader(computeShader.Get(), NULL, 0);

	int threadWidth = ((inputTexture1Desc.Width - 1) / threadWidthInShader) + 1;
	int threadHeight = ((inputTexture1Desc.Height - 1) / threadHeightInShader) + 1;
	int threadLength = ((inputTexture1Desc.Depth - 1) / threadLengthInShader) + 1;
	context->Dispatch(threadWidth, threadHeight, threadLength);

	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetShaderResources(1, 1, nullSRV);
	context->CSSetShaderResources(2, 1, nullSRV);
	
	return;
}


void CalculateHist2D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize) {
	
	ComPtr<ID3D11UnorderedAccessView> uavOutput;
	ComPtr<ID3D11ComputeShader> computeShader;
	ComPtr<ID3D11ShaderResourceView> srvInput1;
	ComPtr<ID3D11ShaderResourceView> srvInput2;
	ComPtr<ID3D11Buffer> threadsParameters;
	ComPtr<ID3D11Buffer> textureParameters;

	//check resolution of input textures
	CD3D11_TEXTURE3D_DESC inputTexture1Desc;
	inputTexture1->GetDesc(&inputTexture1Desc);
	CD3D11_TEXTURE3D_DESC inputTexture2Desc;
	inputTexture2->GetDesc(&inputTexture2Desc);
	if (inputTexture1Desc.Width != inputTexture2Desc.Width) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Height != inputTexture2Desc.Height) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Depth != inputTexture2Desc.Depth) std::invalid_argument("different resolution of input textures");

	CD3D11_TEXTURE2D_DESC outputDesc;
	ZeroMemory(&outputDesc, sizeof(outputDesc));
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.Width = outputSize;
	outputDesc.Height = outputSize;
	outputDesc.Format = DXGI_FORMAT_R32_UINT;
	outputDesc.MipLevels = 1;
	outputDesc.MiscFlags = 0;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ArraySize = 1;
	outputDesc.SampleDesc.Count = 1;	
	

	uint32_t* data = new uint32_t[pow(outputSize, 2)];

	ZeroMemory(data, sizeof(uint32_t)* pow(outputSize, 2));

	int channels = 1;
	int bytesPerChannel = 4;
	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = data;
	subData.SysMemPitch = outputSize * bytesPerChannel;
	subData.SysMemSlicePitch = 0;

	//create View
	DX::ThrowIfFailed(device->CreateTexture2D(&outputDesc, &subData, outputTexture));
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(*outputTexture, nullptr, uavOutput.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture1, nullptr, srvInput1.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture2, nullptr, srvInput2.ReleaseAndGetAddressOf()));

	//load shader
	ID3D10Blob* fileData;
	D3DReadFileToBlob(L"CSHist2D.cso", &fileData);
	DX::ThrowIfFailed(device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, computeShader.ReleaseAndGetAddressOf()));

	//find min max values
	float* minValueInputTex1 = new float();
	float* maxValueInputTex1 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture1, device, context, minValueInputTex1, maxValueInputTex1);

	float* minValueInputTex2 = new float();
	float* maxValueInputTex2 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture2, device, context, minValueInputTex2, maxValueInputTex2);


	//constant buffers
	struct sThreadsParameters {
		int threadWidth;
		int threadHeight;
		int threadLength;
		float padding;
	} bThreadsParameters;

	struct sTextureParameters
	{
		float minScalarValue;
		float maxScalarValue;
		float minFirstDerivValue;
		float maxFirstDerivValue;
		float outputTextureSize;
		DirectX::XMFLOAT3 padding;
	} bTextureParameters;

	int threadWidthInShader = 32;
	int threadHeightInShader = 32;
	int threadLengthInShader = 1;
	bThreadsParameters.threadWidth = inputTexture1Desc.Width;
	bThreadsParameters.threadHeight = inputTexture1Desc.Height;
	bThreadsParameters.threadLength = inputTexture1Desc.Depth;

	bTextureParameters.minScalarValue = *minValueInputTex1;
	bTextureParameters.maxScalarValue = *maxValueInputTex1;
	bTextureParameters.minFirstDerivValue = *minValueInputTex2;
	bTextureParameters.maxFirstDerivValue = *maxValueInputTex2;
	bTextureParameters.outputTextureSize = outputSize;

	CD3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(bThreadsParameters);
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem = &bThreadsParameters;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, threadsParameters.ReleaseAndGetAddressOf());

	bufferDesc.ByteWidth = sizeof(bTextureParameters);

	cbInitData.pSysMem = &bTextureParameters;
	cbInitData.SysMemPitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, textureParameters.ReleaseAndGetAddressOf());

	//Set resoucrs & dispatch
	context->CSSetConstantBuffers(0, 1, threadsParameters.GetAddressOf());
	context->CSSetConstantBuffers(1, 1, textureParameters.GetAddressOf());
	context->CSSetShaderResources(0, 1, srvInput1.GetAddressOf());
	context->CSSetShaderResources(1, 1, srvInput2.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uavOutput.GetAddressOf(), nullptr);
	context->CSSetShader(computeShader.Get(), NULL, 0);

	int threadWidth = ((inputTexture1Desc.Width - 1) / threadWidthInShader) + 1;
	int threadHeight = ((inputTexture1Desc.Height - 1) / threadHeightInShader) + 1;
	int threadLength = ((inputTexture1Desc.Depth - 1) / threadLengthInShader) + 1;
	context->Dispatch(threadWidth, threadHeight, threadLength);

	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetShaderResources(1, 1, nullSRV);
	delete[] data;

	return;

}


void CalculateHist2D(ID3D11Texture3D* inputTexture1, ID3D11Texture3D* inputTexture2,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int outputSize,
	float* maxScalarOut, float* maxGradMagnOut) {

	ComPtr<ID3D11UnorderedAccessView> uavOutput;
	ComPtr<ID3D11ComputeShader> computeShader;
	ComPtr<ID3D11ShaderResourceView> srvInput1;
	ComPtr<ID3D11ShaderResourceView> srvInput2;
	ComPtr<ID3D11Buffer> threadsParameters;
	ComPtr<ID3D11Buffer> textureParameters;

	//check resolution of input textures
	CD3D11_TEXTURE3D_DESC inputTexture1Desc;
	inputTexture1->GetDesc(&inputTexture1Desc);
	CD3D11_TEXTURE3D_DESC inputTexture2Desc;
	inputTexture2->GetDesc(&inputTexture2Desc);
	if (inputTexture1Desc.Width != inputTexture2Desc.Width) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Height != inputTexture2Desc.Height) std::invalid_argument("different resolution of input textures");
	if (inputTexture1Desc.Depth != inputTexture2Desc.Depth) std::invalid_argument("different resolution of input textures");

	CD3D11_TEXTURE2D_DESC outputDesc;
	ZeroMemory(&outputDesc, sizeof(outputDesc));
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.Width = outputSize;
	outputDesc.Height = outputSize;
	outputDesc.Format = DXGI_FORMAT_R32_UINT;
	outputDesc.MipLevels = 1;
	outputDesc.MiscFlags = 0;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ArraySize = 1;
	outputDesc.SampleDesc.Count = 1;


	uint32_t* data = new uint32_t[pow(outputSize, 2)];

	ZeroMemory(data, sizeof(uint32_t) * pow(outputSize, 2));

	int channels = 1;
	int bytesPerChannel = 4;
	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = data;
	subData.SysMemPitch = outputSize * bytesPerChannel;
	subData.SysMemSlicePitch = 0;

	//create View
	DX::ThrowIfFailed(device->CreateTexture2D(&outputDesc, &subData, outputTexture));
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(*outputTexture, nullptr, uavOutput.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture1, nullptr, srvInput1.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(inputTexture2, nullptr, srvInput2.ReleaseAndGetAddressOf()));

	//load shader
	ID3D10Blob* fileData;
	D3DReadFileToBlob(L"CSHist2D.cso", &fileData);
	DX::ThrowIfFailed(device->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, computeShader.ReleaseAndGetAddressOf()));

	//find min max values
	float* minValueInputTex1 = new float();
	float* maxValueInputTex1 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture1, device, context, minValueInputTex1, maxValueInputTex1);

	float* minValueInputTex2 = new float();
	float* maxValueInputTex2 = new float();
	FindMinMaxValuesInTexure3D<float>(inputTexture2, device, context, minValueInputTex2, maxValueInputTex2);

	*maxScalarOut = *maxValueInputTex1;
	*maxGradMagnOut = *maxValueInputTex2;

	//constant buffers
	struct sThreadsParameters {
		int threadWidth;
		int threadHeight;
		int threadLength;
		float padding;
	} bThreadsParameters;

	struct sTextureParameters
	{
		float minScalarValue;
		float maxScalarValue;
		float minFirstDerivValue;
		float maxFirstDerivValue;
		float outputTextureSize;
		DirectX::XMFLOAT3 padding;
	} bTextureParameters;

	int threadWidthInShader = 32;
	int threadHeightInShader = 32;
	int threadLengthInShader = 1;
	bThreadsParameters.threadWidth = inputTexture1Desc.Width;
	bThreadsParameters.threadHeight = inputTexture1Desc.Height;
	bThreadsParameters.threadLength = inputTexture1Desc.Depth;

	bTextureParameters.minScalarValue = *minValueInputTex1;
	bTextureParameters.maxScalarValue = *maxValueInputTex1;
	bTextureParameters.minFirstDerivValue = *minValueInputTex2;
	bTextureParameters.maxFirstDerivValue = *maxValueInputTex2;
	bTextureParameters.outputTextureSize = outputSize;

	CD3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(bThreadsParameters);
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem = &bThreadsParameters;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, threadsParameters.ReleaseAndGetAddressOf());

	bufferDesc.ByteWidth = sizeof(bTextureParameters);

	cbInitData.pSysMem = &bTextureParameters;
	cbInitData.SysMemPitch = 0;

	device->CreateBuffer(&bufferDesc, &cbInitData, textureParameters.ReleaseAndGetAddressOf());

	//Set resoucrs & dispatch
	context->CSSetConstantBuffers(0, 1, threadsParameters.GetAddressOf());
	context->CSSetConstantBuffers(1, 1, textureParameters.GetAddressOf());
	context->CSSetShaderResources(0, 1, srvInput1.GetAddressOf());
	context->CSSetShaderResources(1, 1, srvInput2.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, uavOutput.GetAddressOf(), nullptr);
	context->CSSetShader(computeShader.Get(), NULL, 0);

	int threadWidth = ((inputTexture1Desc.Width - 1) / threadWidthInShader) + 1;
	int threadHeight = ((inputTexture1Desc.Height - 1) / threadHeightInShader) + 1;
	int threadLength = ((inputTexture1Desc.Depth - 1) / threadLengthInShader) + 1;
	context->Dispatch(threadWidth, threadHeight, threadLength);

	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	context->CSSetShaderResources(0, 1, nullSRV);
	context->CSSetShaderResources(1, 1, nullSRV);
	delete[] data;

	return;

}

//Short variant
void CalculateHist3D(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context,
	int histSize) {
	ComPtr<ID3D11Texture3D> firstDerivTexture;
	ComPtr<ID3D11Texture3D> secondDerivTexture;
	CalculateGradientMagnitude6(inputTexture, firstDerivTexture.ReleaseAndGetAddressOf(), device, context);
	CalculateSecondDerivativeAlongGradient(inputTexture, secondDerivTexture.ReleaseAndGetAddressOf(),device, context);
	CalculateHist3D(inputTexture, firstDerivTexture.Get(), secondDerivTexture.Get(), outputTexture, device, context, histSize);
	return;
}
//With returning first derivative magnitude and second derivative along gradient magnitude
void CalculateHist3D(ID3D11Texture3D* inputTexture, ID3D11Texture3D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context,
	int histSize, ID3D11Texture3D** firstDerivTex, ID3D11Texture3D** secondDerivTex) {
	CalculateGradientMagnitude6(inputTexture, firstDerivTex, device, context);
	CalculateSecondDerivativeAlongGradient(inputTexture, secondDerivTex, device, context);
	CalculateHist3D(inputTexture, *firstDerivTex, *secondDerivTex, outputTexture, device, context, histSize);
	return;
}
//Short variant
void CalculateHist2D(ID3D11Texture3D* inputTexture,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int histSize) {

	ComPtr<ID3D11Texture3D> firstDerivTexture;
	CalculateGradientMagnitude6(inputTexture, firstDerivTexture.ReleaseAndGetAddressOf(), device, context);
	CalculateHist2D(inputTexture, firstDerivTexture.Get(), outputTexture, device, context, histSize);
}

void CalculateHist2D(ID3D11Texture3D* inputTexture,
	ID3D11Texture2D** outputTexture, ID3D11Device* device, ID3D11DeviceContext* context, int histSize,
	float* maxScalarOut, float* maxGradMagnOut) {

	ComPtr<ID3D11Texture3D> firstDerivTexture;
	CalculateGradientMagnitude26(inputTexture, firstDerivTexture.ReleaseAndGetAddressOf(), device, context);
	CalculateHist2D(inputTexture, firstDerivTexture.Get(), outputTexture, device, context, histSize, maxScalarOut, maxGradMagnOut);
}

//With returning first derivative magnitude
void CalculateHist2D(ID3D11Texture3D* inputTexture, ID3D11Texture2D** outputTexture, ID3D11Device* device,
	ID3D11DeviceContext* context, int histSize, ID3D11Texture3D** firstDerivTexture) {

	CalculateGradientMagnitude6(inputTexture, firstDerivTexture, device, context);
	CalculateHist2D(inputTexture, *firstDerivTexture, outputTexture, device, context, histSize);
}

void IsSurface(float* pScalarData, DirectX::XMFLOAT4* pGradData, uint16_t* SurefaceIdData, DirectX::XMINT3* texCoord,
	float* prevScalarValue, DirectX::XMFLOAT4* prevGrad, int* width, int* height, int* depth, int* id, int* surfaceSize, int* thresholdChecker) {
	DirectX::XMINT3 newCoord = *texCoord;

	int checker = 0;

	if ((*surfaceSize) > 5) return;

	//coef
	float minKScalarValue = 0.9f;
	float maxKScalarValue = 1.1f;
	float minKGradValue = 0.9f;
	float maxKGradValue = 1.1f;
	//
	float dotComp = 0.1f;

	DirectX::XMFLOAT4 grad;
	float scalarValue;
	//step +x
	if ((*texCoord).x < (*width)-1) {
		newCoord.x += 1;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)]==0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {

				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
	}
	//step -x
	if ((*texCoord).x >= 1) {
		newCoord.x -= 2;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] == 0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {

				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
		newCoord.x += 1;
	}
	//step +y
	if ((*texCoord).y < *height-1) {
		newCoord.y += 1;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] == 0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {

				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
	}
	//step -y
	if ((*texCoord).y >= 1) {
		newCoord.y -= 2;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] == 0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {

				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
		newCoord.y += 1;
	}
	//step +z
	if ((*texCoord).z < *depth-1) {
		newCoord.z += 1;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] == 0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {

				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
	}
	//step -z
	if ((*texCoord).z >= 1) {
		newCoord.z -= 2;
		if (SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] == 0) {

			scalarValue = pScalarData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];
			grad = pGradData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)];

			if (scalarValue > minKScalarValue* (*prevScalarValue) && scalarValue < maxKScalarValue * (*prevScalarValue)
				&& XMFLOAT3Dot(&grad, prevGrad)>dotComp, XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(prevGrad)
				&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(prevGrad)) {
				
				IsSurface(pScalarData, pGradData, SurefaceIdData, &newCoord, &scalarValue, &grad, width, height, depth,
					id, surfaceSize, thresholdChecker);
				++(*surfaceSize);
			}
		}
		newCoord.z += 1;
	}

	if (*surfaceSize > 30) SurefaceIdData[newCoord.x + newCoord.y * (*width) + newCoord.z * (*width) * (*height)] = *id;


}
float XMFLOAT3GetLength(DirectX::XMFLOAT3* input)
{
	return sqrt(input->x * input->x + input->y * input->y + input->z * input->z);
}
float XMFLOAT3Dot(DirectX::XMFLOAT3* input1, DirectX::XMFLOAT3* input2)
{
	return input1->x * input2->x + input1->y * input2->y + input1->z * input2->z;
}

float XMFLOAT3GetLength(DirectX::XMFLOAT4* input)
{
	return sqrt(input->x * input->x + input->y * input->y + input->z * input->z);
}

float XMFLOAT3Dot(DirectX::XMFLOAT4* input1, DirectX::XMFLOAT4* input2)
{
	return input1->x * input2->x + input1->y * input2->y + input1->z * input2->z;
}

DirectX::XMFLOAT3 XMFLOAT3Normalize(DirectX::XMFLOAT3* input1) {
	DirectX::XMFLOAT3 output;
	output.x = input1->x / XMFLOAT3GetLength(input1);
	output.y = input1->y / XMFLOAT3GetLength(input1);
	output.z = input1->z / XMFLOAT3GetLength(input1);
	return output;
}

DirectX::XMFLOAT3 XMFLOAT3Normalize(DirectX::XMFLOAT4* input1) {
	DirectX::XMFLOAT3 output;
	output.x = input1->x / XMFLOAT3GetLength(input1);
	output.y = input1->y / XMFLOAT3GetLength(input1);
	output.z = input1->z / XMFLOAT3GetLength(input1);
	return output;
}

void CheckVoxelsForNeighborhoods(std::vector<DirectX::XMINT3>* oldVoxels, std::vector<DirectX::XMINT3>* newVoxels, 
	float* pScalarData, DirectX::XMFLOAT4* pGradData, bool* tmpIdData, bool* isChecked, int* surfaceSize,
	int16_t width, int16_t height, int16_t depth) {

	for (auto it = oldVoxels->begin(); it != oldVoxels->end(); ++it) {
		DirectX::XMINT3 texCoord = (*it);
		if (!isChecked[texCoord.x + texCoord.y * width + texCoord.z * width * height]) {
			int counter = 0;

			std::vector<DirectX::XMINT3> tmpXVec;
			std::vector<DirectX::XMINT3> tmpYVec;
			std::vector<DirectX::XMINT3> tmpZVec;
			std::vector<DirectX::XMINT3> tmpVec;
			//coef
			float minKScalarValue = 0.7f;
			float maxKScalarValue = 1.3f;
			float minKGradValue = 0.70f;
			float maxKGradValue = 1.35f;
			float dotMax = 1.0f;
			float dotMin = 0.0f;
			float dotNormalsMin = 0.0f;
			//
			DirectX::XMFLOAT4 grad;
			float scalarValue;
			
			float prevScalarValue = pScalarData[texCoord.x + texCoord.y * width + texCoord.z * width * height];
			DirectX::XMFLOAT4 prevGrad = pGradData[texCoord.x + texCoord.y * width + texCoord.z * width * height];
			DirectX::XMINT3 newCoord = texCoord;

			
			//get interior direction
			DirectX::XMFLOAT3 intDir = DirectX::XMFLOAT3(0, 0, 0);
			if (abs(prevGrad.x) > abs(prevGrad.y) && abs(prevGrad.x) > abs(prevGrad.z)) {
				intDir.x = 1;
			}
			else if(abs(prevGrad.y) > abs(prevGrad.z)) {
				intDir.y = 1;
			}
			else {
				intDir.z = 1;
			}
			
			DirectX::XMFLOAT4 pXGrad;
			DirectX::XMFLOAT4 nXGrad;
			DirectX::XMFLOAT4 pYGrad;
			DirectX::XMFLOAT4 nYGrad;
			DirectX::XMFLOAT4 pZGrad;
			DirectX::XMFLOAT4 nZGrad;
			/*
			//step +x
			if (texCoord.x < width - 1) {
				newCoord.x += 1;
				pXGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.x == 0) {
					tmpXVec.push_back(newCoord);
				}
				newCoord.x -= 1;
			}
			//step -x
			if (texCoord.x >= 1) {
				newCoord.x -= 1;
				nXGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.x == 0) {
					tmpXVec.push_back(newCoord);
				}
				newCoord.x += 1;
			}
			//step +y
			if (texCoord.y < height - 1) {
				newCoord.y += 1;
				pYGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.y == 0) {
					tmpYVec.push_back(newCoord);
				}
				newCoord.y -= 1;
			}
			//step -y
			if (texCoord.y >= 1) {
				newCoord.y -= 1;
				nYGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.y == 0) {
					tmpYVec.push_back(newCoord);
				}
				newCoord.y += 1;
			}
			//step +z
			if (texCoord.z < depth - 1) {
				newCoord.z += 1;
				pZGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.z == 0) {
					tmpZVec.push_back(newCoord);
				}
				newCoord.z -= 1;
			}
			//step -z
			if (texCoord.z >= 1) {
				newCoord.z -= 1;
				nZGrad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false
					&& intDir.z == 0) {
					tmpZVec.push_back(newCoord);
				}
				newCoord.z += 1;
			}
			
			if (intDir.x != 0) {
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pYGrad), &XMFLOAT3Normalize(&nYGrad))>dotMin
					&& XMFLOAT3GetLength(&pYGrad) > XMFLOAT3GetLength(&nYGrad)*minKGradValue
					&& XMFLOAT3GetLength(&pYGrad) < XMFLOAT3GetLength(&nYGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pZGrad), &XMFLOAT3Normalize(&nZGrad)) > dotMin
					&& XMFLOAT3GetLength(&pZGrad) > XMFLOAT3GetLength(&nZGrad)* minKGradValue
					&& XMFLOAT3GetLength(&pZGrad) < XMFLOAT3GetLength(&nZGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pXGrad), &XMFLOAT3Normalize(&nXGrad)) > dotNormalsMin) {
					counter++;
				}
			}

			if (intDir.y != 0) {
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pXGrad), &XMFLOAT3Normalize(&nXGrad)) > dotMin
					&& XMFLOAT3GetLength(&pXGrad) > XMFLOAT3GetLength(&nXGrad)* minKGradValue
					&& XMFLOAT3GetLength(&pXGrad) < XMFLOAT3GetLength(&nXGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pZGrad), &XMFLOAT3Normalize(&nZGrad)) > dotMin
					&& XMFLOAT3GetLength(&pZGrad) > XMFLOAT3GetLength(&nZGrad)* minKGradValue
					&& XMFLOAT3GetLength(&pZGrad) < XMFLOAT3GetLength(&nZGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pYGrad), &XMFLOAT3Normalize(&nYGrad)) > dotNormalsMin) {
					counter++;
				}
			}

			if (intDir.z != 0) {
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pYGrad), &XMFLOAT3Normalize(&nYGrad)) > dotMin
					&& XMFLOAT3GetLength(&pYGrad) > XMFLOAT3GetLength(&nYGrad)* minKGradValue
					&& XMFLOAT3GetLength(&pYGrad) < XMFLOAT3GetLength(&nYGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pXGrad), &XMFLOAT3Normalize(&nXGrad)) > dotMin
					&& XMFLOAT3GetLength(&pXGrad) > XMFLOAT3GetLength(&nXGrad)* minKGradValue
					&& XMFLOAT3GetLength(&pXGrad) < XMFLOAT3GetLength(&nXGrad)* maxKGradValue) {
					counter++;
				}
				if (XMFLOAT3Dot(&XMFLOAT3Normalize(&pZGrad), &XMFLOAT3Normalize(&nZGrad)) > dotNormalsMin) {
					counter++;
				}
			}

			if (counter >= 2) {
				tmpIdData[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;
				if (intDir.x != 0) {
					for (auto it2 = tmpXVec.begin(); it2 != tmpXVec.end(); ++it2) {
						//isChecked[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpYVec.begin(); it2 != tmpYVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpZVec.begin(); it2 != tmpZVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					newVoxels->insert(newVoxels->end(), tmpYVec.begin(), tmpYVec.end());
					newVoxels->insert(newVoxels->end(), tmpZVec.begin(), tmpZVec.end());

					newVoxels->insert(newVoxels->end(), tmpXVec.begin(), tmpXVec.end());
				}
				else if (intDir.y != 0) {
					for (auto it2 = tmpXVec.begin(); it2 != tmpXVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpYVec.begin(); it2 != tmpYVec.end(); ++it2) {
						//isChecked[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpZVec.begin(); it2 != tmpZVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					newVoxels->insert(newVoxels->end(), tmpXVec.begin(), tmpXVec.end());
					newVoxels->insert(newVoxels->end(), tmpZVec.begin(), tmpZVec.end());

					newVoxels->insert(newVoxels->end(), tmpYVec.begin(), tmpYVec.end());
				}
				else {
					for (auto it2 = tmpXVec.begin(); it2 != tmpXVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpYVec.begin(); it2 != tmpYVec.end(); ++it2) {
						tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					for (auto it2 = tmpZVec.begin(); it2 != tmpZVec.end(); ++it2) {
						//isChecked[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
					}
					newVoxels->insert(newVoxels->end(), tmpXVec.begin(), tmpXVec.end());
					newVoxels->insert(newVoxels->end(), tmpYVec.begin(), tmpYVec.end());

					newVoxels->insert(newVoxels->end(), tmpZVec.begin(), tmpZVec.end());
				}				
				++(*surfaceSize);
				
			}

			isChecked[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;
			*/
			
			/*
			//step +x
			if (texCoord.x < width - 1) {
				newCoord.x += 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
					&& dot >= dotMin&& dot<=dotMax&& XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}
				newCoord.x -= 1;
			}
			//step -x
			if (texCoord.x >= 1) {
				newCoord.x -= 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
					&& dot>=dotMin && dot <= dotMax && XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}
				newCoord.x += 1;
			}
			//step +y
			if (texCoord.y < height - 1) {
				newCoord.y += 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
					&& dot>=dotMin && dot <= dotMax && XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}
				newCoord.y -= 1;
			}
			//step -y
			if (texCoord.y >= 1) {
				newCoord.y -= 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
					&& dot>=dotMin && dot <= dotMax &&  XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}

				newCoord.y += 1;
			}
			//step +z
			if (texCoord.z < depth - 1) {
				newCoord.z += 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
					&& dot>=dotMin && dot <= dotMax &&  XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}
				newCoord.z -= 1;
			}
			//step -z
			if (texCoord.z >= 1) {
				newCoord.z -= 1;

				scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
				grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

				float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));
				if (scalarValue > minKScalarValue* prevScalarValue && scalarValue < maxKScalarValue * prevScalarValue
					&& dot>=dotMin && dot <= dotMax &&  XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
					&& XMFLOAT3GetLength(&grad) > 1000.0f) {

					if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
						tmpVec.push_back(newCoord);

					}
					++counter;
				}

				newCoord.z += 1;
			}

			if (counter >= 4) {
				tmpIdData[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;
				//for (auto it2 = tmpVec.begin(); it2 != tmpVec.end(); ++it2) {					
					//tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
				//}
				++(*surfaceSize);
				newVoxels->insert(newVoxels->end(), tmpVec.begin(), tmpVec.end());
			}

			isChecked[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;*/

			int minX = -1;	
			int maxX = 1;
			int minY = -1;
			int maxY = 1;
			int minZ = -1;
			int maxZ = 1;

			if (texCoord.x == 0) minX = 0;
			if (texCoord.x == width) maxX = 0;
			if (texCoord.y == 0) minY = 0;
			if (texCoord.y == height) maxY = 0;
			if (texCoord.z == 0) minZ = 0;
			if (texCoord.z == depth) maxZ = 0;

			for (int z = minZ; z <= maxZ; ++z) {
				for (int y = minZ; y <= maxY; ++y) {
					for (int x = minX; x <= maxX; ++x) {
						newCoord = texCoord;
						newCoord.x += x;
						newCoord.y += y;
						newCoord.z += z;

						scalarValue = pScalarData[newCoord.x + newCoord.y * width + newCoord.z * width * height];
						grad = pGradData[newCoord.x + newCoord.y * width + newCoord.z * width * height];

						float dot = XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&prevGrad));

						if (scalarValue > minKScalarValue* prevScalarValue&& scalarValue < maxKScalarValue * prevScalarValue
							&& dot >= dotMin && dot <= dotMax && XMFLOAT3GetLength(&grad) > minKGradValue* XMFLOAT3GetLength(&prevGrad)
							&& XMFLOAT3GetLength(&grad) < maxKGradValue* XMFLOAT3GetLength(&prevGrad)
							&& XMFLOAT3GetLength(&grad) > 1000.0f) {

							if (isChecked[newCoord.x + newCoord.y * width + newCoord.z * width * height] == false) {
								tmpVec.push_back(newCoord);

							}
							++counter;
						}
					}
				}
			}


			if (counter >= 8) {
				tmpIdData[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;
				//for (auto it2 = tmpVec.begin(); it2 != tmpVec.end(); ++it2) {					
					//tmpIdData[(*it2).x + (*it2).y * width + (*it2).z * width * height] = true;
				//}
				++(*surfaceSize);
				newVoxels->insert(newVoxels->end(), tmpVec.begin(), tmpVec.end());
			}

			isChecked[texCoord.x + texCoord.y * width + texCoord.z * width * height] = true;



		}
	}
}

void FindSurfaces(ID3D11Texture3D* scalarTexture, ID3D11Texture3D* gradTexture, ID3D11Texture3D* outputTexture, 
	ID3D11Device* device, ID3D11DeviceContext* context)
{
	//check desc
	D3D11_TEXTURE3D_DESC desc1;
	D3D11_TEXTURE3D_DESC desc2;
	scalarTexture->GetDesc(&desc1);
	gradTexture->GetDesc(&desc2);
	if (desc1.Format != DXGI_FORMAT_R32_FLOAT) {
		Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([]() {
			Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(L"Input 1 text is not R32 format"));
			msg->ShowAsync(); }));
		return;
	}
	if (desc2.Format != DXGI_FORMAT_R32G32B32A32_FLOAT) {
		Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([]() {
			Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(L"Input 2 text is not R32G32B32A32 format"));
			msg->ShowAsync(); }));
		return;
	}
	if (desc1.Width != desc2.Width || desc1.Height != desc2.Height || desc1.Depth != desc2.Depth) {
		Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([]() {
			Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(L"Surface finding texture size error"));
			msg->ShowAsync(); }));
		return;
	}
	//copy data into textures 2d with other parameters
	desc1.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc1.Usage = D3D11_USAGE_STAGING;
	desc1.BindFlags = 0;
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	Microsoft::WRL::ComPtr<ID3D11Texture3D> ScalarTextureCopy;
	Microsoft::WRL::ComPtr<ID3D11Texture3D> GradTextureCopy;
	device->CreateTexture3D(&desc1, NULL, ScalarTextureCopy.ReleaseAndGetAddressOf());
	device->CreateTexture3D(&desc2, NULL, GradTextureCopy.ReleaseAndGetAddressOf());
	context->CopyResource(ScalarTextureCopy.Get(), scalarTexture);
	context->CopyResource(GradTextureCopy.Get(), gradTexture);

	//map the resource
	D3D11_MAPPED_SUBRESOURCE mappedResourceScalar;
	D3D11_MAPPED_SUBRESOURCE mappedResourceGrad;
	ZeroMemory(&mappedResourceScalar, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&mappedResourceGrad, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(ScalarTextureCopy.Get(), 0, D3D11_MAP_READ, 0, &mappedResourceScalar);
	context->Map(GradTextureCopy.Get(), 0, D3D11_MAP_READ, 0, &mappedResourceGrad);
	float* pScalarData = (float*)mappedResourceScalar.pData;
	DirectX::XMFLOAT4* pGradData = (DirectX::XMFLOAT4*)mappedResourceGrad.pData;

	//create buffer for id data
	uint16_t* SurfaceIdData = new uint16_t [desc1.Width * desc1.Height * desc1.Depth];
	ZeroMemory(SurfaceIdData, sizeof(uint16_t)* desc1.Width * desc1.Height * desc1.Depth);
	bool* isChecked = new bool[desc1.Width * desc1.Height * desc1.Depth];
	ZeroMemory(isChecked, sizeof(bool) * desc1.Width * desc1.Height * desc1.Depth);
	bool* tmpIdData = new bool[desc1.Width * desc1.Height * desc1.Depth];
	ZeroMemory(tmpIdData, sizeof(bool) * desc1.Width * desc1.Height * desc1.Depth);

	DirectX::XMFLOAT4* tmpData = new DirectX::XMFLOAT4[desc1.Width * desc1.Height * desc1.Depth];
	ZeroMemory(tmpData, sizeof(DirectX::XMFLOAT4) * desc1.Width * desc1.Height * desc1.Depth);
	int currentId = 1;

	for (int z = 0; z < desc1.Depth; ++z) {
		for (int y = 0; y < desc1.Height; ++y) {
			for (int x = 0; x < desc1.Width; ++x) {
				int ch = 0;
				float prevGradMagnitude = XMFLOAT3GetLength(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]);
				DirectX::XMFLOAT3 step = XMFLOAT3Normalize(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]);
				DirectX::XMINT3 newCoord = DirectX::XMINT3(x, y, z);
				//isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = true;
				newCoord.x += round(step.x);
				newCoord.y += round(step.y);
				newCoord.z += round(step.z);
				{
					if (newCoord.x < 0) newCoord.x = 0;
					if (newCoord.x >= desc1.Width) newCoord.x = desc1.Width - 1;
					if (newCoord.y < 0) newCoord.y = 0;
					if (newCoord.y >= desc1.Height) newCoord.y = desc1.Height - 1;
					if (newCoord.z < 0) newCoord.z = 0;
					if (newCoord.z >= desc1.Depth) newCoord.z = desc1.Depth - 1;
				}
				float newGradMagnitude = XMFLOAT3GetLength(&pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]);
				if (newGradMagnitude < prevGradMagnitude) ++ch;
				newCoord = DirectX::XMINT3(x, y, z);
				//isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = true;
				newCoord.x -= round(step.x);
				newCoord.y -= round(step.y);
				newCoord.z -= round(step.z);
				{
					if (newCoord.x < 0) newCoord.x = 0;
					if (newCoord.x >= desc1.Width) newCoord.x = desc1.Width - 1;
					if (newCoord.y < 0) newCoord.y = 0;
					if (newCoord.y >= desc1.Height) newCoord.y = desc1.Height - 1;
					if (newCoord.z < 0) newCoord.z = 0;
					if (newCoord.z >= desc1.Depth) newCoord.z = desc1.Depth - 1;
				}
				newGradMagnitude = XMFLOAT3GetLength(&pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]);
				if (newGradMagnitude < prevGradMagnitude) ++ch;
				if (ch == 2 && prevGradMagnitude > 250.0f) tmpIdData[x + y * desc1.Width + z * desc1.Width * desc1.Height] = true;
			
			}
		}
	}

	//уберем лишнее
	for (int z3 = 0; z3 < desc1.Depth; ++z3) {
		for (int y3 = 0; y3 < desc1.Height; ++y3) {
			for (int x3 = 0; x3 < desc1.Width; ++x3) {
				int minX = -1;
				int maxX = 1;
				int minY = -1;
				int maxY = 1;
				int minZ = -1;
				int maxZ = 1;
				if (x3 == 0) minX = 0;
				if (x3 == desc1.Width - 1) maxX = 0;
				if (y3 == 0) minY = 0;
				if (y3 == desc1.Height - 1) maxY = 0;
				if (z3 == 0) minZ = 0;
				if (z3 == desc1.Depth - 1) maxZ = 0;

				//float sumGrad = 0;
				//float sumV = abs(minX) + abs(maxX) + abs(minY) + abs(maxY) + abs(minZ) + abs(maxZ);

				int counter28 = 0;

				for (int z2 = minZ; z2 <= maxZ; ++z2) {
					for (int y2 = minY; y2 <= maxY; ++y2) {
						for (int x2 = minX; x2 <= maxX; ++x2) {
							DirectX::XMINT3 newCoord = DirectX::XMINT3(x3, y3, z3);
							newCoord.x += x2;
							newCoord.y += y2;
							newCoord.z += z2;
							bool sampl = tmpIdData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height];
							if (sampl)++ counter28;							
						}
					}
				}
				if (counter28 < 8 
					|| !tmpIdData[x3 + y3 * desc1.Width + z3 * desc1.Width * desc1.Height]) 
					isChecked[x3 + y3 * desc1.Width + z3 * desc1.Width * desc1.Height] = true;
			}
		}
	}

	//алг поиска по пути
	for (int z = 1; z < 6; ++z) {
		for (int y = 0; y < desc1.Height; ++y) {
			for (int x = 0; x < desc1.Width; ++x) {
				if (pScalarData[x + y * desc1.Width + z * desc1.Width * desc1.Height] > 10.0f
					&& isChecked[x + y * desc1.Width + z * desc1.Width * desc1.Height] == false
					&& XMFLOAT3GetLength(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]) > 250.0f
					) {

						{
							std::vector<DirectX::XMINT3> oldVoxels;
							std::vector<DirectX::XMINT3> newVoxels;
							int surfaceSize = 0;
							oldVoxels.push_back(DirectX::XMINT3(x, y, z));
							while (oldVoxels.size() > 0) {
								for (auto it3 = oldVoxels.begin(); it3 != oldVoxels.end(); ++it3) {
									int minX = -1;
									int maxX = 1;
									int minY = -1;
									int maxY = 1;
									int minZ = -1;
									int maxZ = 1;
									if ((*it3).x == 0) minX = 0;
									if ((*it3).x == desc1.Width - 1) maxX = 0;
									if ((*it3).y == 0) minY = 0;
									if ((*it3).y == desc1.Height - 1) maxY = 0;
									if ((*it3).z == 0) minZ = 0;
									if ((*it3).z == desc1.Depth - 1) maxZ = 0;
									float targetMagnitude = XMFLOAT3GetLength(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]);
									float targetScalar = pScalarData[x + y * desc1.Width + z * desc1.Width * desc1.Height];
									DirectX::XMFLOAT4 vec = pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height];
									//float sumGrad = 0;
									//float sumV = abs(minX) + abs(maxX) + abs(minY) + abs(maxY) + abs(minZ) + abs(maxZ);

									for (int z2 = minZ; z2 <= maxZ; ++z2) {
										for (int y2 = minY; y2 <= maxY; ++y2) {
											for (int x2 = minX; x2 <= maxX; ++x2) {
												DirectX::XMINT3 newCoord = (*it3);
												newCoord.x += x2;
												newCoord.y += y2;
												newCoord.z += z2;
												DirectX::XMFLOAT4 grad = pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height];
												float currLength = XMFLOAT3GetLength(&grad);
												float currScalar = pScalarData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height];
												//sumGrad += currLength;

												if (currLength > 0.5f * targetMagnitude && currLength < targetMagnitude/0.5f
													//currScalar > 0.94f*targetScalar && currScalar < 1.1f * targetScalar
													&& currLength > 250.0f
													&& !isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]
													&& XMFLOAT3Dot(&XMFLOAT3Normalize(&grad), &XMFLOAT3Normalize(&vec)) > 0.0f
													) {

													if (SurfaceIdData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] == 0) {

														SurfaceIdData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = currentId;
														newVoxels.push_back(DirectX::XMINT3(newCoord.x, newCoord.y, newCoord.z));
														targetMagnitude = targetMagnitude - (targetMagnitude - currLength) / 2.0f;
													}
												}
											}
										}
									}
								}
								oldVoxels = newVoxels;
								newVoxels.clear();
							}
							++currentId;
						}
				}
			}
		}
	}

	/*for (int z = 0; z < 4; ++z) {
		for (int y = 0; y < desc1.Height; ++y)  {
			for (int x = 0; x < desc1.Width; ++x) {
				if (pScalarData[x+y* desc1.Width +z* desc1.Width * desc1.Height] > 10.0f  
					&& isChecked[x + y * desc1.Width + z * desc1.Width * desc1.Height] == false
					&& XMFLOAT3GetLength (&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height])>1000.0f
					){

						{
							int counter = 0;
							float prevGradMagnitude = XMFLOAT3GetLength(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]);
							DirectX::XMFLOAT3 step = XMFLOAT3Normalize(&pGradData[x + y * desc1.Width + z * desc1.Width * desc1.Height]);
							DirectX::XMINT3 newCoord = DirectX::XMINT3(x, y, z);
							isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = true;
							newCoord.x += round(step.x);
							newCoord.y += round(step.y);
							newCoord.z += round(step.z);
							{
								if (newCoord.x < 0) newCoord.x = 0;
								if (newCoord.x >= desc1.Width) newCoord.x = desc1.Width - 1;
								if (newCoord.y < 0) newCoord.y = 0;
								if (newCoord.y >= desc1.Height) newCoord.y = desc1.Height - 1;
								if (newCoord.z < 0) newCoord.z = 0;
								if (newCoord.z >= desc1.Depth) newCoord.z = desc1.Depth - 1;
							}
							float nextGradMagnitude = XMFLOAT3GetLength(&pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]);
							while (nextGradMagnitude > prevGradMagnitude) {
								isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = true;
								prevGradMagnitude = nextGradMagnitude;
								DirectX::XMFLOAT3 step = XMFLOAT3Normalize(&pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]);
								newCoord.x += round(step.x);
								newCoord.y += round(step.y);
								newCoord.z += round(step.z);
								{
									if (newCoord.x < 0) newCoord.x = 0;
									if (newCoord.x >= desc1.Width) newCoord.x = desc1.Width-1;
									if (newCoord.y < 0) newCoord.y = 0;
									if (newCoord.y >= desc1.Height) newCoord.y = desc1.Height - 1;
									if (newCoord.z < 0) newCoord.z = 0;
									if (newCoord.z >= desc1.Depth) newCoord.z = desc1.Depth - 1;
								}
								nextGradMagnitude = XMFLOAT3GetLength(&pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]);
							}
							isChecked[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = true;

							std::vector<DirectX::XMINT3> oldVoxels;
							std::vector<DirectX::XMINT3> newVoxels;
							int surfaceSize = 0;
							int c = 0;
							int nums2 = 4;
							oldVoxels.push_back(DirectX::XMINT3(newCoord.x, newCoord.y, newCoord.z));
							float targetMagnitude = prevGradMagnitude;
							while (oldVoxels.size() > 0) {
								for (auto it3 = oldVoxels.begin(); it3 != oldVoxels.end(); ++it3) {
									int minX = -1;
									int maxX = 1;
									int minY = -1;
									int maxY = 1;
									int minZ = -1;
									int maxZ = 1;
									if ((*it3).x == 0) minX = 0;
									if ((*it3).x == desc1.Width-1) maxX = 0;
									if ((*it3).y == 0) minY = 0;
									if ((*it3).y == desc1.Height-1) maxY = 0;
									if ((*it3).z == 0) minZ = 0;
									if ((*it3).z == desc1.Depth-1) maxZ = 0;

									//float sumGrad = 0;
									//float sumV = abs(minX) + abs(maxX) + abs(minY) + abs(maxY) + abs(minZ) + abs(maxZ);

									for (int z2 = minZ; z2 <= maxZ; ++z2) {
										for (int y2 = minY; y2 <= maxY; ++y2) {
											for (int x2 = minX; x2 <= maxX; ++x2) {
												newCoord = (*it3);
												newCoord.x += x2;
												newCoord.y += y2;
												newCoord.z += z2;
												DirectX::XMFLOAT4 grad = pGradData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height];
												float currLength = XMFLOAT3GetLength(&grad);
												//sumGrad += currLength;

												if (currLength >0.82f * targetMagnitude && currLength < 1.2f * targetMagnitude
													&& currLength > 1000.0f) {

													if (SurfaceIdData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height]==0) {
														SurfaceIdData[newCoord.x + newCoord.y * desc1.Width + newCoord.z * desc1.Width * desc1.Height] = currentId;
														newVoxels.push_back(DirectX::XMINT3(newCoord.x, newCoord.y, newCoord.z));
														if(nums2<=64) targetMagnitude = targetMagnitude - (targetMagnitude - currLength) / nums2 / 2.0f;
													}
												}
											}
										}
									}									
								}
								oldVoxels = newVoxels;
								newVoxels.clear();
								nums2*=2.0f;
							}
							++currentId;
							
						}
						*/

					/*std::vector<DirectX::XMINT3> oldVoxels;
					std::vector<DirectX::XMINT3> newVoxels;
					int surfaceSize = 0;
					int c = 0;

					oldVoxels.push_back(DirectX::XMINT3(x,y,z));

					while (oldVoxels.size() != 0) {
						CheckVoxelsForNeighborhoods(&oldVoxels, &newVoxels, pScalarData, pGradData,
							tmpIdData, isChecked, &surfaceSize, desc1.Width, desc1.Height, 20);
						oldVoxels = newVoxels;
						newVoxels.clear();
						c++;
					}

					if (surfaceSize > 15) {
						for (int i = 0; i < desc1.Width * desc1.Height * desc1.Depth; ++i) {
							if(tmpIdData[i]==true) SurfaceIdData[i] = currentId;
							
						}
						++currentId;
					}

					ZeroMemory(tmpIdData, sizeof(bool) * desc1.Width * desc1.Height * desc1.Depth);*/
				//}
				
			//}
		//}		
	//}
	/*for (int i = 0; i < desc1.Width * desc1.Height * desc1.Depth; ++i) {
		SurfaceIdData[i] = isChecked[i];
	}*/

	D3D11_TEXTURE3D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Format = DXGI_FORMAT_R16_UINT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = desc1.Width;
	desc.Height = desc1.Height;
	desc.Depth = desc1.Depth;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = SurfaceIdData;
	subData.SysMemPitch = desc.Width*sizeof(uint16_t);
	subData.SysMemSlicePitch = desc.Width * desc.Height * sizeof(uint16_t);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> idTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvIdTexture;
	device->CreateTexture3D(&desc, &subData, idTexture.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(idTexture.Get(), NULL, srvIdTexture.ReleaseAndGetAddressOf());

	context->HSSetShaderResources(2, 1, srvIdTexture.GetAddressOf());


	//show colors test
	DirectX::XMFLOAT3* colors = new DirectX::XMFLOAT3[currentId];
	colors[0].x = 0;
	colors[0].y = 0;
	colors[0].z = 0;
	for (int i = 1; i <= currentId; ++i) {
		colors[i].x = (rand() % 1000)/1000.0f;
		colors[i].y = (rand() % 1000)/1000.0f;
		colors[i].z = (rand() % 1000)/1000.0f;
	}

	DirectX::XMFLOAT4* colorsData = new DirectX::XMFLOAT4[desc1.Width * desc1.Height * desc1.Depth];
	ZeroMemory(colorsData, sizeof(DirectX::XMFLOAT4) * desc1.Width * desc1.Height * desc1.Depth);

	for (int i = 0; i < desc1.Width * desc1.Height * desc1.Depth; ++i) {
		uint16_t id = SurfaceIdData[i];
		colorsData[i].x = colors[id].x;
		colorsData[i].y = colors[id].y;
		colorsData[i].z = colors[id].z;
	}

	ZeroMemory(&desc, sizeof(desc));
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = desc1.Width;
	desc.Height = desc1.Height;
	desc.Depth = desc1.Depth;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;

	subData.pSysMem = colorsData;
	subData.SysMemPitch = desc.Width * sizeof(DirectX::XMFLOAT4);
	subData.SysMemSlicePitch = desc.Width * desc.Height * sizeof(DirectX::XMFLOAT4);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> colorsTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvColorsTexture;
	device->CreateTexture3D(&desc, &subData, colorsTexture.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(colorsTexture.Get(), NULL, srvColorsTexture.ReleaseAndGetAddressOf());

	context->HSSetShaderResources(3, 1, srvColorsTexture.GetAddressOf());

	//test start
	for (int i = 0; i < desc1.Width * desc1.Height * desc1.Depth; ++i) {
		if (SurfaceIdData[i] != 0) tmpData[i] = pGradData[i];
	}

	ZeroMemory(&desc, sizeof(desc));
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = desc1.Width;
	desc.Height = desc1.Height;
	desc.Depth = desc1.Depth;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;

	subData.pSysMem = tmpData;
	subData.SysMemPitch = desc.Width * sizeof(DirectX::XMFLOAT4);
	subData.SysMemSlicePitch = desc.Width * desc.Height * sizeof(DirectX::XMFLOAT4);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> tmTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTmTex;
	device->CreateTexture3D(&desc, &subData, tmTex.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(tmTex.Get(), NULL, srvTmTex.ReleaseAndGetAddressOf());

	context->HSSetShaderResources(1, 1, srvTmTex.GetAddressOf());

	//test end
	delete[] colorsData;

	delete[] SurfaceIdData;

	delete[] tmpIdData;

	delete[] isChecked;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv3test;
	DX::CreateTextureSRV(scalarTexture, srv3test.GetAddressOf(), device);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv4test;
	DX::CreateTextureSRV(gradTexture, srv4test.GetAddressOf(), device);

	context->HSSetShaderResources(4, 1, srv3test.GetAddressOf());
	context->HSSetShaderResources(5, 1, srv4test.GetAddressOf());

	ZeroMemory(&desc, sizeof(desc));
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = desc1.Width;
	desc.Height = desc1.Height;
	desc.Depth = desc1.Depth;
	desc.CPUAccessFlags = 0;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;

	subData;
	subData.pSysMem = pScalarData;
	subData.SysMemPitch = desc.Width * sizeof(float);
	subData.SysMemSlicePitch = desc.Width * desc.Height * sizeof(float);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> ttt;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvttt;
	device->CreateTexture3D(&desc, &subData, ttt.ReleaseAndGetAddressOf());
	device->CreateShaderResourceView(ttt.Get(), NULL, srvttt.ReleaseAndGetAddressOf());
	context->HSSetShaderResources(6, 1, srvttt.GetAddressOf());
}


template <class T>
void FindMinMaxValuesInTexure3D(ID3D11Texture3D* inputTexture, ID3D11Device* device, ID3D11DeviceContext* context, T* outputMinValue, T* outputMaxValue) {

	D3D11_MAPPED_SUBRESOURCE mappedSub;
	CD3D11_TEXTURE3D_DESC inputTexDesc;
	ComPtr<ID3D11Texture3D> stagingTexture;
	inputTexture->GetDesc(&inputTexDesc);

	CD3D11_TEXTURE3D_DESC stagingTexDesc = inputTexDesc;
	stagingTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTexDesc.Usage = D3D11_USAGE_STAGING;
	stagingTexDesc.BindFlags = 0;

	DX::ThrowIfFailed(device->CreateTexture3D(&stagingTexDesc, NULL, stagingTexture.ReleaseAndGetAddressOf()));
	context->CopyResource(stagingTexture.Get(), inputTexture);

	uint32_t inputWidth = inputTexDesc.Width;
	uint32_t inputHeight = inputTexDesc.Height;
	uint32_t inputLength = inputTexDesc.Depth;
	if (inputWidth + inputHeight + inputLength < 1) std::invalid_argument("Input texture size less than 1");

	DX::ThrowIfFailed(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedSub));

	T* pointToValue = new T[inputWidth * inputHeight * inputLength];

	pointToValue = (T*)mappedSub.pData;
	T minValue = pointToValue[0];
	T maxValue = pointToValue[0];
	for (int i = 0; i < int(inputWidth * inputHeight * inputLength); ++i) {
		if (minValue > pointToValue[i]) minValue = pointToValue[i];
		else if (maxValue < pointToValue[i]) maxValue = pointToValue[i];
	}

	*outputMinValue = minValue;
	*outputMaxValue = maxValue;

	context->Unmap(stagingTexture.Get(), 0);

	return;
}

template <class T>
void FindMinMaxValuesInTexure2D(ID3D11Texture2D* inputTexture, ID3D11Device* device, ID3D11DeviceContext* context, T* minValue, T* maxValue) {

	D3D11_MAPPED_SUBRESOURCE mappedSub;
	CD3D11_TEXTURE3D_DESC inputTexDesc;
	ComPtr<ID3D11Texture3D> stagingTexture;
	inputTexture->GetDesc(&inputTexDesc);

	CD3D11_TEXTURE2D_DESC stagingTexDesc = inputTexDesc;
	stagingTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTexDesc.Usage = D3D11_USAGE_STAGING;
	stagingTexDesc.BindFlags = 0;

	DX::ThrowIfFailed(device->CreateTexture2D(&stagingTexDesc, NULL, stagingTexture.ReleaseAndGetAddressOf()));
	context->CopyResource(stagingTexture.Get(), inputTexture);

	uint32_t inputWidth = inputTexDesc.Width;
	uint32_t inputHeight = inputTexDesc.Height;
	if (inputWidth + inputHeight < 1) std::invalid_argument("Input texture size less than 1");

	DX::ThrowIfFailed(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedSub));

	T* pointToValue = new T[inputWidth * inputHeight];

	pointToValue = (T*)mappedSub.pData;
	T minValue = pointToValue[0];
	T maxValue = pointToValue[0];
	for (int i = 0; i < int(inputWidth * inputHeight); ++i) {
		if (minValue > pointToValue[i]) minValue = pointToValue[i];
		else if (maxValue < pointToValue[i]) maxValue = pointToValue[i];
	}

	*outputMinValue = minValue;
	*outputMaxValue = maxValue;

	context->Unmap(stagingTexture.Get(), 0);

	return;

}

