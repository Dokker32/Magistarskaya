#include "pch.h"
#include "InterfaceElement.h"

#define greyScale 0.375f

using namespace DirectX;

bool InterfaceElement::Initialize(uint16_t pixelHight, uint16_t pixelWidth, 
	const std::shared_ptr<DX::DeviceResources>& deviceResources, float maxValue, float* histoData, int numColumns) {

	HRESULT HR;
	this->deviceResources = deviceResources;

	this->maxValue = maxValue;
	this->pixelHight = pixelHight;
	this->pixelWidth = pixelWidth*3;	

	HR = this->LoadShaders(L"VInterfaceShader.cso", L"PInterfaceShader.cso");
	this->createBackground(maxValue, histoData, numColumns);

	loadingComplete = true;

	return true;
}

bool InterfaceElement::LoadShaders(const std::wstring& vShaderName, const std::wstring& pShaderName) {
	// Асинхронная загрузка шейдеров.
	auto loadVSTask = DX::ReadDataAsync(vShaderName);
	auto loadPSTask = DX::ReadDataAsync(pShaderName);

	// После загрузки файла шейдера вершин создаются шейдер и входной макет.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&inputLayout
			)
		);
	});

	// После загрузки файла шейдера пикселей создаются шейдер и буфер констант.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&pixelShader
			)
		);
	});


	auto createSquadTask = (createPSTask && createVSTask).then([this]() {
		static const IESquad squadVertices[] =
		{
			{XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f)},
			{XMFLOAT3(0.5f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f)},
			{XMFLOAT3(-1.0f,  -0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f)},
			{XMFLOAT3(-1.0f,  -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f)}
		};
		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = squadVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(squadVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&squadVertexBuffer
			)
		);

		static const unsigned short squadIndices[] =
		{
			0,1,2,1,3,2
		};

		squadIndexCount = ARRAYSIZE(squadIndices);


		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = squadIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(squadIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&squadIndexBuffer
			)
		);

	});

	// После загрузки куба объект готов к отрисовке.
	createSquadTask.then([this]() {
		loadingComplete = true;
	});
	return true;
}


bool InterfaceElement::Render() {
	if (loadingComplete) {
		auto context = deviceResources->GetD3DDeviceContext();

		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader.Get(), nullptr, 0);
		context->PSSetShaderResources(3, 1, background.GetAddressOf());
		UINT stride = sizeof(IESquad);
		UINT offset = 0;

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(inputLayout.Get());
		context->IASetVertexBuffers(0, 1, squadVertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(squadIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		context->DrawIndexed(squadIndexCount, 0, 0);

		return true;
	}

	return false;
}

bool InterfaceElement::createBackground(float maxValue, float* histoData, int numColumns) {

	uint16_t cWidth = (uint16_t)this->pixelWidth;
	uint16_t cHeight = (uint16_t)this->pixelHight;

	D3D11_TEXTURE2D_DESC texureDesc = {};
	texureDesc.Width = cWidth;
	texureDesc.Height = cHeight;
	texureDesc.ArraySize = 1;
	texureDesc.MipLevels = 1;
	texureDesc.SampleDesc.Count = 1;
	texureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texureDesc.Usage = D3D11_USAGE_DEFAULT;
	texureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texureDesc.CPUAccessFlags = 0;
	texureDesc.MiscFlags = 0;

	const uint16_t bytesPerChannel = 4;
	const uint16_t channels = 4;
	if (sizeof(float) != 4) return false;
	float* tex3d = new float[texureDesc.Width * texureDesc.Height * channels];
	ZeroMemory(tex3d, texureDesc.Width * texureDesc.Height * bytesPerChannel * channels);

	//закрашиваем задний фон
	for (uint16_t y = 0; y < cHeight; y++)
	{
		for (uint16_t x = 0; x < cWidth; x++)
		{
			// Add some dummy color
			tex3d[0 + x * channels + y * cWidth * channels] = 0.169f;
			tex3d[1 + x * channels + y * cWidth * channels] = 0.169f;
			tex3d[2 + x * channels + y * cWidth * channels] = 0.169f;
			tex3d[3 + x * channels + y * cWidth * channels] = 1.0f; //alpha
		}
	}

	//CreateAxes(tex3d, numColumns); //error
	CreateHisto(tex3d, maxValue, histoData, numColumns);

	D3D11_SUBRESOURCE_DATA initData = { 0 };

	initData.pSysMem = tex3d;
	initData.SysMemPitch = cWidth * bytesPerChannel * channels; //in bytes.
	initData.SysMemSlicePitch = 0;

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex = nullptr;
	hr = deviceResources->GetD3DDevice()->CreateTexture2D(&texureDesc, &initData, &tex);

	hr = deviceResources->GetD3DDevice()->CreateShaderResourceView(tex.Get(), NULL, background.GetAddressOf());
	delete[] tex3d;

	return true;
}

bool InterfaceElement::CreateAxes(float* tex3d, int numColumns) {
	int XOffcet = this->pixelWidth/20;
	int YOffcet = this->pixelHight/10;
	this->XAxesPixelOffset = XOffcet;
	this->YAxesPixelOffset = YOffcet;
	int plotPixelHeight = this->pixelHight - this->YAxesPixelOffset * 2;
	int plotPixelWidth = this->pixelWidth - this->XAxesPixelOffset * 2;
	int channels = 4;
	int a;

	//XAxes
	for (int i = XOffcet; i <= (pixelWidth - XOffcet); ++i) {
		tex3d[0 + this->pixelWidth * channels * (pixelHight - YOffcet) + i * channels] = greyScale;
		tex3d[1 + this->pixelWidth * channels * (pixelHight - YOffcet) + i * channels] = greyScale;
		tex3d[2 + this->pixelWidth * channels * (pixelHight - YOffcet) + i * channels] = greyScale;
		a = i;
	}

	//YAxes
	for (int i = YOffcet; i <= (pixelHight - YOffcet); ++i) {
		tex3d[0 + XOffcet * channels + i * this->pixelWidth * channels] = greyScale;
		tex3d[1 + XOffcet * channels + i * this->pixelWidth * channels] = greyScale;
		tex3d[2 + XOffcet * channels + i * this->pixelWidth * channels] = greyScale;
		a = i;
	}

	//засечки
	int lineSize = 9; // in pixels
	if (lineSize / 2 > this->YAxesPixelOffset || lineSize / 2 > this->XAxesPixelOffset) return false;

	//X axes
	float step = 20; //in Pixels
	for (int i = XOffcet + step/2.0f; i <= plotPixelWidth + XOffcet; i+=step) {
		for (int j = plotPixelHeight + YOffcet - lineSize / 2.0; j <= plotPixelHeight + YOffcet + lineSize / 2.0; ++j) {
			tex3d[0 + i * channels + j * this->pixelWidth * channels] = greyScale;
			tex3d[1 + i * channels + j * this->pixelWidth * channels] = greyScale;
			tex3d[2 + i * channels + j * this->pixelWidth * channels] = greyScale;
		}
	}
	//Y axes
	//step = plotPixelHeight / 10;
	for (int i = plotPixelHeight + YOffcet - step / 2.0f; i >= YOffcet; i -= step) {
		for (int j = XOffcet - lineSize / 2.0; j <= XOffcet + lineSize / 2.0; ++j) {
			tex3d[0 + j * channels + i * this->pixelWidth * channels] = greyScale;
			tex3d[1 + j * channels + i * this->pixelWidth * channels] = greyScale;
			tex3d[2 + j * channels + i * this->pixelWidth * channels] = greyScale;
		}
	}

	
	return true;
}

bool InterfaceElement::CreateHisto(float* tex3d, float maxValue, float* histoData, int columnsNum) {
	int channels = 4;
	float columnWidth = (this->pixelWidth - this->XAxesPixelOffset * 2)/ (float)columnsNum;
	float  YMaxValue = maxValue;
	int plotPixelHeight = this->pixelHight - this->YAxesPixelOffset * 2;
	int maxNumValues = 0;

	for (int i = 0; i < columnsNum; ++i) {
		if (maxNumValues < histoData[i]) maxNumValues = histoData[i];
	}

	int maxNumValues2 = 0;
	int count;
	for (int i = 0; i < columnsNum; ++i) {
		if (maxNumValues2 < histoData[i]) {
			maxNumValues2 = histoData[i];
			count = i;
		}
	}
	float hist0 = histoData[count];
	float hist1 = histoData[count + 1];
	float hist2 = histoData[count + 2];
	float hist3 = histoData[count + 3];
	float hist4 = histoData[count + 4];
	float hist5 = histoData[count + 5];
	float hist6 = histoData[count + 6];
	float hist7 = histoData[count + 7];
	float hist8 = histoData[count + 8];
	float hist9 = histoData[count + 9];
	float hist10 = histoData[count + 10];
	float hist11 = histoData[count + 11];


	//float XAxesPixelOffset = this->XAxesPixelOffset + 1; //иначе закрашивает ось.
	for (int i = 0; i < columnsNum; ++i) {
		//в ширину
		for (int j = XAxesPixelOffset + (int)(i * columnWidth); j < XAxesPixelOffset + (int)((i+1) * columnWidth); ++j) {
			//в высоту
			for(int k = YAxesPixelOffset + plotPixelHeight - plotPixelHeight*(histoData[i]/maxNumValues); k< plotPixelHeight + YAxesPixelOffset; ++k){
				//костыль, чтобы не перезаписывало оси.
				if (tex3d[0 + j * channels + k * this->pixelWidth * channels] != greyScale) {
					tex3d[0 + j * channels + k * this->pixelWidth * channels] = 0.11f;
					tex3d[1 + j * channels + k * this->pixelWidth * channels] = 0.11f;
					tex3d[2 + j * channels + k * this->pixelWidth * channels] = 0.11f;
				}
			}			
		}
	}

	return true;

}