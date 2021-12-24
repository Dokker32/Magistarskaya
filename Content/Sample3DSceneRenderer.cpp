#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include <d3dcompiler.h>
#include "BlankPage.xaml.h"
#include <ctime>

using namespace MRIRenderRCT;

using namespace DirectX;
using namespace Windows::Foundation;

// Загружает шейдеры вершин и пикселей из файлов и создает геометрию куба.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources),
	dicomFile("MR2"),
	isRealTimeRender(true),
	MRITextureReady(false),
	TF2DScaleStruct(1,0,0,100),
	tissueFiguresManager(m_deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Инициализирует параметры представления при изменении размера окна.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Это простой пример изменения, которое можно реализовать, когда приложение находится в
	// книжном или прикрепленном представлении.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Обратите внимание, что матрица OrientationTransform3D подвергается здесь дополнительному умножению,
	// чтобы правильно сориентировать сцену в соответствии с ориентацией экрана.
	// Это умножение является обязательным для всех вызовов draw,
	// предназначенных для целевого объекта в цепочке буферов. В случае вызовов draw для других целевых объектов
	// это преобразование не должно применяться.

	// В этом примере используется правая система координат на базе матриц с развертыванием по столбцам.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Наблюдатель находится в точке (0,0.7,1.5) и смотрит в точку (0,-0.1,0), а вектор вертикали направлен вдоль оси Y.
	static const XMVECTORF32 eye = { 0.0f, -1.5f, 0.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, -0.1f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	this->InitializationRenderStates();

	//prepare srv and other
	outputTextureSizeX = (int)m_deviceResources->getRenderTargetSize().Width;
	outputTextureSizeY = (int)m_deviceResources->getRenderTargetSize().Height;
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, mainSampler.GetAddressOf());
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, notInterSampler.GetAddressOf());
	

	CD3D11_TEXTURE2D_DESC textureDesc;	
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Height = outputTextureSizeY;
	textureDesc.Width = outputTextureSizeX;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.MiscFlags = 0;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	CD3D11_TEXTURE2D_DESC textureDesc2 = textureDesc;
	float textureChannels = 4;
	float* textureResultData = new float[textureDesc.Height * textureDesc.Width * textureChannels];	

	m_deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc, NULL, renderResult.ReleaseAndGetAddressOf());
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	m_deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc, NULL, mFront.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateTexture2D(&textureDesc, NULL, mBack.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(mFront.Get(), NULL, srvFront.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(mBack.Get(), NULL, srvBack.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(renderResult.Get(), NULL, srvRenderResult.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateUnorderedAccessView(renderResult.Get(), NULL, uavRenderResult.ReleaseAndGetAddressOf());
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	m_deviceResources->GetD3DDevice()->CreateRenderTargetView(mFront.Get(), &rtvDesc, &rtvFront);
	m_deviceResources->GetD3DDevice()->CreateRenderTargetView(mBack.Get(), &rtvDesc, &rtvBack);

	MRIParamStructInst.mousePosX = 20;
	MRIParamStructInst.mousePosX = 20;
	MRIParamStructInst.zMRITextureSize = 230;
	MRIParamStructInst.yResultTextureSize = (int)m_deviceResources->getRenderTargetSize().Height;
	MRIParamStructInst.xResultTextureSize = (int)m_deviceResources->getRenderTargetSize().Width;
	MRIParamStructInst.maxScalarValue = this->MRIMaxScalarValue;
	MRIParamStructInst.maxGradMagn = this->MRIMaxGradMagn;
	MRIParamStructInst.numElectrodes = 0;
	MRIParamStructInst.UpdateElectrode = -1;

	//create texture for electrodes (TEST)
	/*CD3D11_TEXTURE1D_DESC texture1DDesc;
	ZeroMemory(&texture1DDesc, sizeof(texture1DDesc));
	texture1DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture1DDesc.CPUAccessFlags = 0;
	texture1DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture1DDesc.Width = 3;
	texture1DDesc.MipLevels = 1;
	texture1DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture1DDesc.MiscFlags = 0;
	texture1DDesc.ArraySize = 1;

	float testData3[9] = { 0.698, 0.31, 0.639, 0.467, 0.231, 0.698, 0.259, 0.314, 0.647 };
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = testData3;
	initData.SysMemPitch = 3 * 3 * sizeof(float);

	m_deviceResources->GetD3DDevice()->CreateTexture1D(&texture1DDesc, &initData, electrodeTexture.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateShaderResourceView(electrodeTexture.Get(), NULL, srvElectrodeTexture.ReleaseAndGetAddressOf());*/

	//create active electrode
	this->numElectrodes = 1;

	CD3D11_TEXTURE1D_DESC texture1DDesc;
	ZeroMemory(&texture1DDesc, sizeof(texture1DDesc));
	texture1DDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	texture1DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texture1DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture1DDesc.Width = this->numElectrodes;
	texture1DDesc.MipLevels = 1;
	texture1DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture1DDesc.MiscFlags = 0;
	texture1DDesc.ArraySize = 1;

	electrodeRawTexture = new float[1 * 4];
	for (int i = 0; i < 4; ++i) {
		electrodeRawTexture[i] = -10000;
	}
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = electrodeRawTexture;

	m_deviceResources->GetD3DDevice()->CreateTexture1D(&texture1DDesc, &initData, electrodeTexture.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateUnorderedAccessView(electrodeTexture.Get(), NULL, uavElectrodeTexture.ReleaseAndGetAddressOf());

	
}

// Вызывается по одному разу для каждого кадра
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Преобразование градусов в радианы с последующим преобразованием секунд в угол поворота
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(0, XM_2PI));

		Rotate();
	}


}

// Поворот модели трехмерного куба на заданный угол в радианах.
void Sample3DSceneRenderer::Rotate()
{
	// Подготовка к передаче обновленной матрицы модели шейдеру
	auto rMatrixX = XMMatrixTranspose(XMMatrixRotationX(this->rotator.x));
	auto rMatrixY = XMMatrixTranspose(XMMatrixRotationY(this->rotator.y));
	auto rMatrixZ = XMMatrixTranspose(XMMatrixRotationZ(this->rotator.z));

	auto rMatrix = XMMatrixMultiply(rMatrixX, rMatrixY);
	rMatrix = XMMatrixMultiply(rMatrix, rMatrixZ);
	//XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationZ(radians)));


	XMStoreFloat4x4(&m_constantBufferData.model, rMatrix);
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// При отслеживании трехмерный куб можно вращать вокруг его оси Y, следя за положением указателя относительно ширины экрана вывода.
void Sample3DSceneRenderer::TrackingUpdate(float positionX, float positionY)
{
	if (m_tracking)
	{
		if (currentSwapChain==MAIN) {
			float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
			Rotate();
		}
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// Прорисовывает один кадр с помощью шейдеров вершин и пикселей.
bool Sample3DSceneRenderer::Render()
{
	if (this->tissueFiguresManager.UpdateFigCom && this->isTF2DCreated) {
		for (auto it = tissueFiguresManager.tissueFigures.begin(); it != tissueFiguresManager.tissueFigures.end(); ++it) {
			if ((*it).points.size() == 3) {
				FillFigure(this->TransferFunction2D.Get(), &(*it), m_deviceResources);
				this->isTFReady = true;
			}
		}
		this->tissueFiguresManager.UpdateFigCom = false;
	}
	if (this->renderState == NEW_TEXTURE_LOADING_RENDER_STATE)
	{
		if (this->createSecond) {
			if (this->createFromRaw) {
				D3D11_TEXTURE3D_DESC textureDesc;
				textureDesc.Width = this->xSizeSecondTextureRaw;
				textureDesc.Height = this->ySizeSecondTextureRaw;
				textureDesc.MipLevels = 1;
				textureDesc.Depth = this->zSizeSecondTextureRaw;
				textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.CPUAccessFlags = 0;
				textureDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA initData = { 0 };
				initData.pSysMem = this->rawTextureData;
				initData.SysMemPitch = this->xSizeSecondTextureRaw * sizeof(float);
				initData.SysMemSlicePitch = this->xSizeSecondTextureRaw * this->ySizeSecondTextureRaw * sizeof(float);

				HRESULT hr = m_deviceResources->GetD3DDevice()->CreateTexture3D(&textureDesc, &initData, secondMRITexture.GetAddressOf());

				createFromRaw = false;
				delete[]  this->rawTextureData;
			}
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(secondMRITexture.Get(), NULL, srvSecondMRITexture.ReleaseAndGetAddressOf());
			ApplyConvolution(secondMRITexture.Get(), secondMRITextureBlurred.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext(), 3, 1.5f);
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(secondMRITextureBlurred.Get(), NULL, srvSecondMRITextureBlurred.ReleaseAndGetAddressOf());
		}
		else {
			if (this->createFromRaw) {
				D3D11_TEXTURE3D_DESC textureDesc;
				textureDesc.Width = this->xSizeTextureRaw;
				textureDesc.Height = this->ySizeTextureRaw;
				textureDesc.MipLevels = 1;
				textureDesc.Depth = this->zSizeTextureRaw;
				textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				textureDesc.CPUAccessFlags = 0;
				textureDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA initData = { 0 };
				initData.pSysMem = this->rawTextureData;
				initData.SysMemPitch = this->xSizeTextureRaw * sizeof(float);
				initData.SysMemSlicePitch = this->xSizeTextureRaw * this->ySizeTextureRaw * sizeof(float);

				HRESULT hr = m_deviceResources->GetD3DDevice()->CreateTexture3D(&textureDesc, &initData, MRITexture.GetAddressOf());

				createFromRaw = false;
				delete[]  this->rawTextureData;
			}
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(MRITexture.Get(), NULL, srvMRITexture.ReleaseAndGetAddressOf());
			ApplyConvolution(MRITexture.Get(), MRITextureBlurred.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext(), 3, 1.5f);
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(MRITextureBlurred.Get(), NULL, srvMRITextureBlurred.ReleaseAndGetAddressOf());
			//unsigned int threadGroupCountX = (((int)m_deviceResources->getRenderTargetSize().Width - 1) / 32) + 1;
			//unsigned int threadGroupCountY = (((int)m_deviceResources->getRenderTargetSize().Height - 1) / 32) + 1;
			//unsigned int threadGroupCountZ = 1;
			//m_deviceResources->GetD3DDeviceContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
			//calc hist
			int histSize = 512;
			CalculateHist2D(MRITextureBlurred.Get(), Hist2D.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext(), histSize, &this->MRIMaxScalarValue, &this->MRIMaxGradMagn);
			CalculateGradient6Points(MRITextureBlurred.Get(), MRIGradTexture.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext());
			CalculateNodeScalar8Points(MRITextureBlurred.Get(), MRIScalarNodeTexture.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext());


			DX::CreateTextureSRV(MRIGradTexture.Get(), srvMRIGradTexture.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice());
			DX::CreateTextureSRV(MRIScalarNodeTexture.Get(), srvMRIScalarNodeTexture.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice());

			MRIParamStructInst.maxScalarValue = this->MRIMaxScalarValue;
			MRIParamStructInst.maxGradMagn = this->MRIMaxGradMagn;

			m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(cbMRIParam.Get(), 0, NULL, &MRIParamStructInst, 0, 0, 0);

			//test
			/*Microsoft::WRL::ComPtr<ID3D11Texture3D> outputTest3;
			CalculateNodeGradient8Points(MRITextureBlurred.Get(), outputTest3.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext());



			Microsoft::WRL::ComPtr<ID3D11Texture3D> outputTest4;
			CalculateNodeScalar8Points(MRITextureBlurred.Get(), outputTest4.ReleaseAndGetAddressOf(), m_deviceResources->GetD3DDevice(),
				m_deviceResources->GetD3DDeviceContext());

			FindSurfaces(outputTest4.Get(), outputTest3.Get(), MRITexture.Get(),
				m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv1test;
			//DX::CreateTextureSRV(MRITextureBlurred.Get(), srv1test.GetAddressOf(), m_deviceResources->GetD3DDevice());
			m_deviceResources->GetD3DDeviceContext()->HSSetShaderResources(0, 1, srv1test.GetAddressOf());
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv2test;
			DX::CreateTextureSRV(MRIGradTexture.Get(), srv2test.GetAddressOf(), m_deviceResources->GetD3DDevice());
			//m_deviceResources->GetD3DDeviceContext()->HSSetShaderResources(1, 1, srv2test.GetAddressOf());*/


			//create transfer function 2D texture with the same size of the hist
			{
				CD3D11_TEXTURE2D_DESC TFDesc;
				ZeroMemory(&TFDesc, sizeof(TFDesc));
				TFDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				TFDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				TFDesc.Width = histSize;
				TFDesc.Height = histSize;
				TFDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				TFDesc.MipLevels = 1;
				TFDesc.MiscFlags = 0;
				TFDesc.Usage = D3D11_USAGE_DYNAMIC;
				TFDesc.ArraySize = 1;
				TFDesc.SampleDesc.Count = 1;

				float* data = new float[pow(histSize, 2) * 4];

				ZeroMemory(data, sizeof(float) * pow(histSize, 2) * 4);

				D3D11_SUBRESOURCE_DATA initData;
				initData.pSysMem = data;
				initData.SysMemPitch = histSize;
				initData.SysMemSlicePitch = 0;

				DX::ThrowIfFailed(this->m_deviceResources->GetD3DDevice()->CreateTexture2D(&TFDesc, &initData, this->TransferFunction2D.ReleaseAndGetAddressOf()));
				m_deviceResources->GetD3DDevice()->CreateShaderResourceView(this->TransferFunction2D.Get(), NULL, this->srvTransferFunction2D.ReleaseAndGetAddressOf());
				this->tissueFiguresManager.TransferFunction2D = this->TransferFunction2D.Get();
				this->tissueFiguresManager.srvTransferFunction2D = this->srvTransferFunction2D.Get();
				this->isTF2DCreated = true;
				delete[] data;
			}

			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(this->Hist2D.Get(), NULL, this->srvHist2D.ReleaseAndGetAddressOf());
		}
		renderState = READY_STATE;
	}
	else if (renderState == READY_STATE){
		// Загрузка является асинхронной. Геометрию можно рисовать только после ее загрузки.
		if (!m_loadingComplete || (!m_DoRender && !isRealTimeRender))
		{
			return false;
		}			
				
		while (addedElectrodes < numElectrodesToAdd) {
			this->AddElectrode();
			++addedElectrodes;
			this->isUpdateMRIParamBuffer = true;
		}
			//this->isAddElectrode = false;

		if (isUpdateElectrode) {
			this->UpdateElectrode();
			this->isUpdateMRIParamBuffer = true;
			isUpdateElectrode = false;
		}
		else if (isTrackingElectrode) {
			this->TrackingElectrode();
			this->isUpdateMRIParamBuffer = true;		
		}
		else {
			MRIParamStructInst.UpdateElectrode = -1;
			this->isUpdateMRIParamBuffer = true;
		}

		//delete test
		/*if (this->start == nullptr) this->start = new std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::high_resolution_clock::now());
		std::chrono::time_point<std::chrono::high_resolution_clock> currTime = std::chrono::high_resolution_clock::now();
		auto dT = currTime - *start;
		float deltaTime = dT.count()/1000000000.0;
		float value = 0;
		float del = 583.0;
		del = 1878824;
		if (deltaTime <= 20) {
			value = 0;
		}
		else if (deltaTime <= 25) {
			value = del / 5.0 * float(deltaTime-20.0f);
		}
		else if (deltaTime <= 30) {
			value = del - del / 5.0 * float(deltaTime-25.0f);
		}
		else {
			value=0;
		}
		this->Sample3DSceneRenderer::UpdateBrainThresholdValue(value);*/
		// /delete

		auto context = m_deviceResources->GetD3DDeviceContext();
		if ((currentSwapChain == TRANSFER_FUNCTION_2D) && (TF2DInitialized == true)) {			
			this->RenderHist();
			this->RenderTF2DAreas();
			if (isRenderPointUnderCursor) {
				this->RenderPointUnderCursor();
			}
			if (instanceCount > 0) {
				this->tissueFiguresManager.Render(cbTF2DScale, cbPSPointStruct);
			}			
			
		}
		if (MRITextureReady && currentSwapChain ==  MAIN) {
			//set IA
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//render cube positions
			this->RenderRCPositions();
			//render into intermediate texture2D
			context->CSSetSamplers(0, 1, mainSampler.GetAddressOf());
			context->CSSetSamplers(1, 1, notInterSampler.GetAddressOf());
			context->CSSetShaderResources(3, 1, srvMRITexture.GetAddressOf());
			context->CSSetShaderResources(1, 1, srvFront.GetAddressOf());
			context->CSSetShaderResources(2, 1, srvBack.GetAddressOf());
			context->CSSetShaderResources(3, 1, srvTransferFunction2D.GetAddressOf());
			context->CSSetShaderResources(4, 1, srvMRIGradTexture.GetAddressOf());
			context->CSSetShaderResources(5, 1, srvHist2D.GetAddressOf());
			context->CSSetShaderResources(7, 1, srvMRIGradTexture.GetAddressOf());			
			context->CSSetUnorderedAccessViews(1, 1, uavElectrodeTexture.GetAddressOf(), nullptr);
			context->CSSetShaderResources(0, 1,srvMRITextureBlurred.GetAddressOf());
			context->CSSetUnorderedAccessViews(0, 1, uavRenderResult.GetAddressOf(), nullptr);
			if (this->isUpdateMRIParamBuffer) {
				m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(cbMRIParam.Get(), 0, NULL, &MRIParamStructInst, 0, 0, 0);
				this->isUpdateMRIParamBuffer = false;
			}
			context->CSSetConstantBuffers(0, 1, cbMRIParam.GetAddressOf());

			if (useSecondTexture) {
				ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
				context->CSSetUnorderedAccessViews(1, 1, nullUAV, 0);
				this->UpdateElectrodeValues();
				context->CSSetShader(CSMRIWithAddTexture.Get(), nullptr, 0);
				context->CSSetShaderResources(6, 1, srvSecondMRITextureBlurred.GetAddressOf());
				context->CSSetUnorderedAccessViews(1, 1, uavElectrodeTexture.GetAddressOf(), nullptr);
			}
			else {
				if (!this->isTFReady) context->CSSetShader(m_computeShader.Get(), nullptr, 0);
				else context->CSSetShader(CSMRITF.Get(), nullptr, 0);
			}

			unsigned int threadGroupCountX = (((int)m_deviceResources->getRenderTargetSize().Width - 1) / 32) + 1;
			unsigned int threadGroupCountY = (((int)m_deviceResources->getRenderTargetSize().Height - 1) / 32) + 1;
			unsigned int threadGroupCountZ = 1;
			context->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

			//unbind resources
			ID3D11Buffer* nullBuf[1] = { nullptr };
			ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
			context->CSSetUnorderedAccessViews(1, 1, nullUAV, 0);
			context->CSSetShaderResources(1, 1, nullSRV);
			context->CSSetShaderResources(2, 1, nullSRV);
			context->CSSetShaderResources(3, 1, nullSRV);
			context->CSSetShaderResources(4, 1, nullSRV);
			context->CSSetShaderResources(5, 1, nullSRV);
			context->CSSetConstantBuffers(0, 1, nullBuf);

			//render result
			UINT stride = sizeof(VertexPositionColor);
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
			context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
			context->IASetInputLayout(m_inputLayout.Get());
			context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
			context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
			context->PSSetShaderResources(0, 1, srvRenderResult.GetAddressOf());
			context->PSSetSamplers(0, 1, mainSampler.GetAddressOf());
			context->DrawIndexed(m_indexCount, 0, 0);			

			//unbind resources
			context->PSSetShaderResources(0, 1, nullSRV);
		}
		this->m_DoRender = false;
	}
		return true;
	
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// Асинхронная загрузка шейдеров.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadCSTask = DX::ReadDataAsync(L"CSMRI.cso");
	auto loadPSCubeTask = DX::ReadDataAsync(L"PSPos.cso");
	auto loadVSCubeTask = DX::ReadDataAsync(L"VSPos.cso");
	auto loadPSPointTask = DX::ReadDataAsync(L"PSPoint.cso");
	auto loadVSPointTask = DX::ReadDataAsync(L"VSPoint.cso");
	auto loadPSPointInstTask = DX::ReadDataAsync(L"PSPointInst.cso");
	auto loadVSPointInstTask = DX::ReadDataAsync(L"VSPointInst.cso");

	ID3DBlob* fileData;
	D3DReadFileToBlob(L"PSAreas.cso", &fileData);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreatePixelShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &PSAreas)
	);
	fileData->Release();

	D3DReadFileToBlob(L"CSMRITF.cso", &fileData);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &CSMRITF)
	);
	fileData->Release();

	fileData;
	D3DReadFileToBlob(L"CSMRIWithAddTex.cso", &fileData);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateComputeShader(fileData->GetBufferPointer(), fileData->GetBufferSize(), NULL, &CSMRIWithAddTexture)
	);
	fileData->Release();

	auto createVSPointTask = loadVSPointTask.then([this](const std::vector<byte>& fileData) {

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&VSPoint
			)
		);

	});
	auto createPSPointTask = loadPSPointTask.then([this](const std::vector<byte>& fileData) {

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&PSPoint
			)
		);

	});
	
	
	auto createPSPointInstTask = loadPSPointInstTask.then([this](const std::vector<byte>& fileData) {

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&PSPointInst
			)
		);

	});

	auto createVSPointInstTask = loadVSPointInstTask.then([this](const std::vector<byte>& fileData) {

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&VSPointInst
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC instanceDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "POSITION", 1, DXGI_FORMAT_R32G32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				instanceDesc,
				ARRAYSIZE(instanceDesc),
				&fileData[0],
				fileData.size(),
				&instanceInputLayout
			)
		);

		static const VertexPositionColor planeVertices[] =
		{
			{XMFLOAT3(-1.0f, -1.0f, 0.0f)},
			{XMFLOAT3(-1.0f, 1.0f,  0.0f)},
			{XMFLOAT3(1.0f,  1.0f, 0.0f)},
			{XMFLOAT3(1.0f,  1.0f,  0.0f)},
			{XMFLOAT3(1.0f,  -1.0f, 0.0f)},
			{XMFLOAT3(-1.0f,  -1.0f,  0.0f)}

		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = planeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(planeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&instanceVertexBuffer
			)
		);


	});


	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(VSPointStruct), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&cbVSPointStuct
		)
	);

	CD3D11_BUFFER_DESC constantBufferDesc2(sizeof(PSPointStruct), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&constantBufferDesc2,
			nullptr,
			&cbPSPointStruct
		)
	);


	auto createVSCubeTask = loadVSCubeTask.then([this](const std::vector<byte>& fileData) {

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShaderPos
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayoutPos
			)
		);

		static const VertexPositionTexcoord cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f),  XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f),  XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f),  XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBufferPos
			)
		);

		static const unsigned short cubeIndices[] =
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCountPos = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferPos
			)
		);

	});

	auto createPSCubeTask = loadPSCubeTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShaderPos
			)
		);
	});

	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{	
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	// После загрузки файла шейдера пикселей создаются шейдер и буфер констант.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
	});

	auto createCSTask = loadCSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateComputeShader(&fileData[0], fileData.size(), nullptr, m_computeShader.ReleaseAndGetAddressOf())
		);

	});

	// После загрузки обоих шейдеров создайте сетку.
	auto createCubeTask = (createPSTask && createVSTask && createCSTask && createPSCubeTask && createVSCubeTask && createPSPointTask &&createVSPointTask && createVSPointInstTask && createPSPointInstTask).then([this]() {

		// Загрузка вершин сетки. У каждой вершины есть позиция и цвет.
		static const VertexPositionColor planeVertices[] =
		{
			{XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f,1.0f)},
			{XMFLOAT3(-1.0f, 1.0f,  0.0f), XMFLOAT2(0.0f,0.0f)},
			{XMFLOAT3(1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f,0.0f)},
			{XMFLOAT3(1.0f,  1.0f,  0.0f), XMFLOAT2(1.0f,0.0f)},
			{XMFLOAT3(1.0f,  -1.0f, 0.0f), XMFLOAT2(1.0f,1.0f) },
			{XMFLOAT3(-1.0f,  -1.0f,  0.0f), XMFLOAT2(0.0f,1.0f)},

		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = planeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(planeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		// Загрузка индексов сетки. Каждая тройка индексов представляет
		// треугольник для прорисовки на экране.
		// Например: 0,2,1 означает, что вершины с индексами
		// 0, 2 и 1 из буфера вершин составляют 
		// первый треугольник этой сетки.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, 
			3,4,5,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);
	});

	// После загрузки куба объект готов к отрисовке.
	createCubeTask.then([this]() {
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

ID3D11Texture3D** Sample3DSceneRenderer::getMRITexturePointer() {	
	return this->MRITexture.ReleaseAndGetAddressOf();
}

void Sample3DSceneRenderer::DoRender() {
	this->m_DoRender = true;
}

void Sample3DSceneRenderer::UpdateMRITextureSRV() {
	renderState = NEW_TEXTURE_LOADING_RENDER_STATE;
}

void Sample3DSceneRenderer::MRITextureSetSizeParameters(unsigned int height, unsigned int width, unsigned int length) {
	MRITextureHeight = height;
	MRITextureLength = length;
	MRITextureWidth = width;

	MRIParamStructInst.mousePosX = 20;
	MRIParamStructInst.mousePosY = 20;
	MRIParamStructInst.yResultTextureSize = (int)m_deviceResources->getRenderTargetSize().Height;
	MRIParamStructInst.xResultTextureSize = (int)m_deviceResources->getRenderTargetSize().Width;
	MRIParamStructInst.xMRITextureSize = width;
	MRIParamStructInst.yMRITextureSize = height;
	MRIParamStructInst.zMRITextureSize = length;

	CD3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(MRIParamStruct);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &MRIParamStructInst;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	m_deviceResources->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData, cbMRIParam.ReleaseAndGetAddressOf());
}

void Sample3DSceneRenderer::RenderRCPositions() {
	UINT stride = sizeof(VertexPositionTexcoord);
	UINT offset = 0;
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	context->VSSetShader(m_vertexShaderPos.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShaderPos.Get(), nullptr, 0);
	context->IASetInputLayout(m_inputLayoutPos.Get());
	context->IASetVertexBuffers(0, 1, m_vertexBufferPos.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBufferPos.Get(), DXGI_FORMAT_R16_UINT, 0);
	float color[4] = { 0.0f,0.0f,0.0f,1.0f };
	context->ClearRenderTargetView(rtvFront.Get(), color);
	context->ClearRenderTargetView(rtvBack.Get(), color);
	context->OMSetRenderTargets(1, rtvBack.GetAddressOf(), m_deviceResources->GetDepthStencilView());
	m_deviceResources->setBackRasterizerState();
	context->DrawIndexed(m_indexCountPos, 0, 0);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH || D3D11_CLEAR_STENCIL, 1, 0);
	context->OMSetRenderTargets(1, rtvFront.GetAddressOf(), m_deviceResources->GetDepthStencilView());
	m_deviceResources->setFrontRasterizerState();
	context->DrawIndexed(m_indexCountPos, 0, 0);

	m_deviceResources->SetMainRenderTarget();
}

void Sample3DSceneRenderer::InitializationRenderStates() {
	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_deviceResources->GetBlendState(), NULL, 0xFFFFFF);
}

void Sample3DSceneRenderer::SetCurrentSwapChain(CurrentSwapChain currentSwapChain) {
	this->currentSwapChain = currentSwapChain;
}

void Sample3DSceneRenderer::RenderHist() {
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(m_inputLayout.Get());
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr); //not uniq shader
	context->PSSetShader(PS2DTF.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, srvHist2D.GetAddressOf());
	context->UpdateSubresource1(cbTF2DScale.Get(), 0, NULL, &TF2DScaleStruct, 0, 0, 0);
	context->PSSetConstantBuffers1(0, 1, cbTF2DScale.GetAddressOf(), nullptr, nullptr);
	context->PSSetSamplers(0, 1, mainSampler.GetAddressOf());
	context->DrawIndexed(m_indexCount, 0, 0);
}

void Sample3DSceneRenderer::RenderTF2DAreas() {
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_deviceResources->DisableDepthTest();
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(m_inputLayout.Get());
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr); //not uniq shader
	context->PSSetShader(PSAreas.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, srvTransferFunction2D.GetAddressOf());
	context->UpdateSubresource1(cbTF2DScale.Get(), 0, NULL, &TF2DScaleStruct, 0, 0, 0);
	context->PSSetConstantBuffers1(0, 1, cbTF2DScale.GetAddressOf(), nullptr, nullptr);
	context->PSSetSamplers(0, 1, mainSampler.GetAddressOf());
	context->DrawIndexed(m_indexCount, 0, 0);
	m_deviceResources->EnableDepthTest();
}

void Sample3DSceneRenderer::Create2DTransferFunctionResources() {
	auto loadPS2DTFTask = DX::ReadDataAsync(L"PS2DTF.cso");

	auto createPS2DTFTask = loadPS2DTFTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				this->PS2DTF.ReleaseAndGetAddressOf()
			)
		);

	});

	auto createConstBuffer = createPS2DTFTask.then([this]() {
		

		CD3D11_BUFFER_DESC cbDesc(sizeof(ScaleStruct), D3D11_BIND_CONSTANT_BUFFER);
		/*ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.ByteWidth = sizeof(ScaleStruct);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;*/

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &TF2DScaleStruct;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		m_deviceResources->GetD3DDevice()->CreateBuffer(&cbDesc, &InitData, cbTF2DScale.ReleaseAndGetAddressOf());

	});

	auto setFlag = createConstBuffer.then([this]() {
		TF2DInitialized = true;
	});

}


void Sample3DSceneRenderer::AddToTF2DScale(float wheel, float wheelPointerLocationX, float wheelPointerLocationY) {

	
	//update scale
	if (TF2DInitialized && wheelPointerLocationX!=-1 && wheelPointerLocationY!=-1) {
		if (wheel > 100) wheel = 100.0f;

		float oldScale = this->TF2DScaleStruct.scale;
		float newScale;
		if (wheel < 0) newScale = this->TF2DScaleStruct.scale / 1.05f;
		else newScale = this->TF2DScaleStruct.scale * 1.05f;
		

		if (newScale > 50) newScale = 50;
		if (newScale < 1) newScale = 1;

		this->TF2DScaleStruct.scale = newScale;


		//update edges
		if (wheel < 0) {
			//to Center
			this->TF2DScaleStruct.xEdge +=  (0.5f - oldScale / newScale / 2) / oldScale;
			this->TF2DScaleStruct.yEdge += (0.5f - oldScale / newScale / 2) / oldScale;
		}

		else {	
			//to Center
			this->TF2DScaleStruct.xEdge +=  (0.5f - oldScale / newScale / 2) / oldScale;
			this->TF2DScaleStruct.yEdge += (0.5f - oldScale / newScale / 2) / oldScale;

		}


		if (this->TF2DScaleStruct.xEdge < 0) this->TF2DScaleStruct.xEdge = 0; 
		if (this->TF2DScaleStruct.xEdge > 1 - 1 / newScale) this->TF2DScaleStruct.xEdge = 1 - (1 / newScale);
		if (this->TF2DScaleStruct.yEdge < 0) this->TF2DScaleStruct.yEdge = 0; 			
		if (this->TF2DScaleStruct.yEdge > 1 - 1 / newScale) this->TF2DScaleStruct.yEdge = 1 - (1 / newScale);

	}

	
}


void Sample3DSceneRenderer::UpdateTF2DEdgesView(float xEdge, float yEdge) {

	float scale = this->TF2DScaleStruct.scale;

	this->TF2DScaleStruct.xEdge += xEdge * 0.01f / scale;
	this->TF2DScaleStruct.yEdge += yEdge * 0.01f / scale / this->m_deviceResources->getRenderTargetSize().Width * this->m_deviceResources->getRenderTargetSize().Height;


	if (this->TF2DScaleStruct.xEdge < 0) this->TF2DScaleStruct.xEdge = 0;
	if (this->TF2DScaleStruct.xEdge > 1 - 1/scale) this->TF2DScaleStruct.xEdge = 1 - (1 / scale);
	if (this->TF2DScaleStruct.yEdge < 0) this->TF2DScaleStruct.yEdge = 0;
	if (this->TF2DScaleStruct.yEdge > 1 - 1/scale) this->TF2DScaleStruct.yEdge = 1 - (1 / scale);


}

void Sample3DSceneRenderer::RenderPointUnderCursor() {
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_deviceResources->DisableDepthTest();

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(m_inputLayout.Get());
	context->VSSetConstantBuffers(0,1,cbVSPointStuct.GetAddressOf());
	context->VSSetShader(VSPoint.Get(), nullptr, 0);
	context->PSSetShader(PSPoint.Get(), nullptr, 0);
	context->PSSetConstantBuffers(0, 1, cbPSPointStruct.GetAddressOf());
	

	context->DrawIndexed(m_indexCount, 0, 0);

	m_deviceResources->EnableDepthTest();
	isRenderPointUnderCursor = false;
}

void Sample3DSceneRenderer::SetPointToRender(float x, float y, Windows::UI::Color color) {
	
	VSPointStructInst.xPosition = x / this->m_deviceResources->GetLogicalSize().Width;
	VSPointStructInst.yPosition = -y / this->m_deviceResources->GetLogicalSize().Height;

	PSPointStructInst.xCenter = x / this->m_deviceResources->GetLogicalSize().Width * this->m_deviceResources->getRenderTargetSize().Width;
	PSPointStructInst.yCenter = y / this->m_deviceResources->GetLogicalSize().Height * this->m_deviceResources->getRenderTargetSize().Height;
	PSPointStructInst.red = color.R / 255.0f;
	PSPointStructInst.green = color.G / 255.0f;
	PSPointStructInst.blue = color.B / 255.0f;

	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(cbPSPointStruct.Get(), 0, NULL, &PSPointStructInst, 0, 0, 0);
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(cbVSPointStuct.Get(), 0, NULL, &VSPointStructInst, 0, 0, 0);

	isRenderPointUnderCursor = true;
}

void Sample3DSceneRenderer::UpdateInstancePointBuffer(std::vector<TissueElement^>* input) {
	int numPoints = 0;
	for (auto it = input->begin(); it != input->end(); ++it) {
		numPoints+=(*it)->GetPointsNum();

	}

	/*Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
		ref new Windows::UI::Core::DispatchedHandler([this, r]() {
		Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(to_wstring(r).c_str()));
		msg->ShowAsync(); }));*/

	
	//prepare buffer data
	instStruct* data = new instStruct[numPoints];
	this->instanceCount = 0;
	for (auto it = input->begin(); it != input->end(); ++it) {
		for(int i = 0; i < (*it)->GetPointsP()->Size; ++i){
			data[this->instanceCount].center.x = (*it)->GetPointsP()->GetAt(i).X / this->m_deviceResources->GetLogicalSize().Width;
			data[this->instanceCount].center.y = 1-(*it)->GetPointsP()->GetAt(i).Y / this->m_deviceResources->GetLogicalSize().Height;
			data[this->instanceCount].color.x = (*it)->GetTissueColor().R/255.0f;
			data[this->instanceCount].color.y = (*it)->GetTissueColor().G/255.0f;
			data[this->instanceCount].color.z = (*it)->GetTissueColor().B/255.0f;

			++this->instanceCount;
		}
	}
	//create new instance buffer
	CD3D11_BUFFER_DESC instBufferDesc;
	ZeroMemory(&instBufferDesc, sizeof(instBufferDesc));
	instBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instBufferDesc.ByteWidth = sizeof(instStruct)* numPoints;
	instBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instBufferDesc.CPUAccessFlags = 0;
	instBufferDesc.MiscFlags = 0;
	instBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	instanceData.pSysMem = data;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	this->m_deviceResources->GetD3DDevice()->CreateBuffer(&instBufferDesc,&instanceData, this->instanceBuffer.ReleaseAndGetAddressOf());
}

void Sample3DSceneRenderer::RenderPointsTF2D() {
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_deviceResources->DisableDepthTest();

	UINT strides[2];
	UINT offsets[2];
	strides[0] = sizeof(VertexPositionColor);
	strides[1] = sizeof(instStruct);
	offsets[0] = 0;
	offsets[1] = 0;
	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = this->instanceVertexBuffer.Get();
	bufferPointers[1] = this->instanceBuffer.Get();
	context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	context->IASetInputLayout(instanceInputLayout.Get());
	context->VSSetConstantBuffers1(0, 1, cbTF2DScale.GetAddressOf(), nullptr, nullptr);
	context->VSSetShader(VSPointInst.Get(), nullptr, 0);
	context->PSSetShader(PSPointInst.Get(), nullptr, 0);
	context->PSSetConstantBuffers(0, 1, cbPSPointStruct.GetAddressOf());

	int vertexCount = 6;
	context->DrawInstanced(vertexCount, this->instanceCount,0, 0);

	m_deviceResources->EnableDepthTest();
}

void TissueFigure2D::AddPoint(float x, float y, std::shared_ptr<DX::DeviceResources> deviceResources)
{	

	//add point
	for (unsigned int i = 0; i< UINT_MAX-1; ++i){		
		bool isFaced = false;
		for (auto it = this->points.begin(); it != this->points.end(); ++it) {
			if ((*it).pointId == i) isFaced = true;
		}
		if (!isFaced) {
			this->points.push_back(TissueFigurePoint(x, y, i));
			i = UINT_MAX - 1;
		}		
	}
	//add line
	if (this->points.size() > 1) {

		for (unsigned int i = 0; i < UINT_MAX - 1; ++i) {
			bool isFaced = false;
			for (auto it = this->lines.begin(); it != this->lines.end(); ++it) {
				if ((*it).lineId == i) isFaced = true;
			}
			if (!isFaced) {
				this->lines.push_back(TissueFigureLine(
					this->points.at(this->points.size() - 2).GetPosition(),
					this->points.at(this->points.size() - 1).GetPosition(),
					i
				));
				i = UINT_MAX - 1;
			}
		}

		//add 2-3 line
		if (this->points.size() == 3) {
			for (unsigned int i = 0; i < UINT_MAX - 1; ++i) {
				bool isFaced = false;
				for (auto it = this->lines.begin(); it != this->lines.end(); ++it) {
					if ((*it).lineId == i) isFaced = true;
				}
				if (!isFaced) {
					this->lines.push_back(TissueFigureLine(
						this->points.at(this->points.size() - 1).GetPosition(),
						this->points.at(0).GetPosition(),
						i
					));
					i = UINT_MAX - 1;
				}
			}
		}
	}

	//update buffers
	this->UpdateBuffers();
}

void TissueFigure2D::UpdateBuffers()
{
	int numPoints = this->points.size();


	if (numPoints > 0) {

		//prepare buffer data
		instStruct* data = new instStruct[numPoints];
		int instanceCount = 0;

		for (auto it = this->points.begin(); it != points.end(); ++it) {
			data[instanceCount].center.x = (((*it).GetPosition().X) + 1.0f) / 2.0f;
			data[instanceCount].center.y = 1 - ((*it).GetPosition().Y + 1.0f) / 2.0f;
			data[instanceCount].color.x = this->color.R/255.0f;
			data[instanceCount].color.y = this->color.G/255.0f;
			data[instanceCount].color.z = this->color.B/255.0f;
			++instanceCount;
		}

		//create new instance buffer
		CD3D11_BUFFER_DESC instBufferDesc;
		ZeroMemory(&instBufferDesc, sizeof(instBufferDesc));
		instBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		instBufferDesc.ByteWidth = sizeof(instStruct) * numPoints;
		instBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instBufferDesc.CPUAccessFlags = 0;
		instBufferDesc.MiscFlags = 0;
		instBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceData;
		instanceData.pSysMem = data;
		instanceData.SysMemPitch = 0;
		instanceData.SysMemSlicePitch = 0;

		deviceResources->GetD3DDevice()->CreateBuffer(&instBufferDesc, &instanceData, this->pointsInstanceBuffer.ReleaseAndGetAddressOf());
		delete[] data;
	}

	if (this->lines.size() > 0) {
		//lines
		CD3D11_BUFFER_DESC linesBufferDesc;
		ZeroMemory(&linesBufferDesc, sizeof(linesBufferDesc));
		linesBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		linesBufferDesc.ByteWidth = sizeof(linesDataStruct) * this->lines.size() * 2;
		linesBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		linesBufferDesc.CPUAccessFlags = 0;
		linesBufferDesc.MiscFlags = 0;
		linesBufferDesc.StructureByteStride = 0;

		//2 points per line
		linesDataStruct* data2 = new linesDataStruct[this->lines.size() * 2];

		int dataCount = 0;
		for (auto it = this->lines.begin(); it != this->lines.end(); ++it) {
			data2[dataCount * 2].position = DirectX::XMFLOAT3((*it).startPos.X, -(*it).startPos.Y, 0.0f);
			data2[dataCount * 2].color.x = this->color.R / 255.0f;
			data2[dataCount * 2].color.y = this->color.G / 255.0f;
			data2[dataCount * 2].color.z = this->color.B / 255.0f;
			data2[dataCount * 2 + 1].position = DirectX::XMFLOAT3((*it).endPos.X, -(*it).endPos.Y, 0.0f);
			data2[dataCount * 2 + 1].color.x = this->color.R / 255.0f;
			data2[dataCount * 2 + 1].color.y = this->color.G / 255.0f;
			data2[dataCount * 2 + 1].color.z = this->color.B / 255.0f;

			dataCount++;
		}

		D3D11_SUBRESOURCE_DATA linesVertexData;
		linesVertexData.pSysMem = data2;
		linesVertexData.SysMemPitch = 0;
		linesVertexData.SysMemSlicePitch = 0;

		deviceResources->GetD3DDevice()->CreateBuffer(&linesBufferDesc, &linesVertexData, this->linesVertexBuffer.ReleaseAndGetAddressOf());
		delete[] data2;
	}
}


TissueFiguresManager::TissueFiguresManager(std::shared_ptr<DX::DeviceResources> deviceResources) : deviceResources(deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();
	//load shader
	ID3D10Blob* fileData;
	D3DReadFileToBlob(L"VSPointInst.cso", &fileData);
	DX::ThrowIfFailed(device->CreateVertexShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, this->VSPointInst.ReleaseAndGetAddressOf()));
	{
		static const D3D11_INPUT_ELEMENT_DESC instanceDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "POSITION", 1, DXGI_FORMAT_R32G32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		DX::ThrowIfFailed(
			device->CreateInputLayout(
				instanceDesc,
				ARRAYSIZE(instanceDesc),
				fileData->GetBufferPointer(),
				fileData->GetBufferSize(),
				&pointLayout
			)
		);

		static const VertexPositionColor planeVertices[] =
		{
			{XMFLOAT3(-1.0f, -1.0f, 0.0f)},
			{XMFLOAT3(-1.0f, 1.0f,  0.0f)},
			{XMFLOAT3(1.0f,  1.0f, 0.0f)},
			{XMFLOAT3(1.0f,  1.0f,  0.0f)},
			{XMFLOAT3(1.0f,  -1.0f, 0.0f)},
			{XMFLOAT3(-1.0f,  -1.0f,  0.0f)}

		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = planeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(planeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			device->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&vertexPlaneBuffer
			)
		);
	}
	fileData->Release();

	D3DReadFileToBlob(L"PSPointInst.cso", &fileData);
	DX::ThrowIfFailed(device->CreatePixelShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, this->PSPointInst.ReleaseAndGetAddressOf()));
	fileData->Release();
	D3DReadFileToBlob(L"VSFigureLines.cso", &fileData);
	DX::ThrowIfFailed(device->CreateVertexShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, this->VSLines.ReleaseAndGetAddressOf()));
	{
		static const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			device->CreateInputLayout(
				inputLayoutDesc,
				ARRAYSIZE(inputLayoutDesc),
				fileData->GetBufferPointer(),
				fileData->GetBufferSize(),
				&linesLayout
			)
		);
	}
	fileData->Release();
	D3DReadFileToBlob(L"PSFigureLines.cso", &fileData);
	DX::ThrowIfFailed(device->CreatePixelShader(fileData->GetBufferPointer(), fileData->GetBufferSize(),
		NULL, this->PSLines.ReleaseAndGetAddressOf()));

}

void TissueFiguresManager::Render(Microsoft::WRL::ComPtr <ID3D11Buffer> cbTF2DScale, Microsoft::WRL::ComPtr <ID3D11Buffer> cbPSPointStuct)
{
	auto device = deviceResources->GetD3DDevice();
	auto context = deviceResources->GetD3DDeviceContext();
	deviceResources->DisableDepthTest();
	for (auto it = this->tissueFigures.begin(); it != this->tissueFigures.end(); ++it) {
		if ((*it).GetPointsNum() > 0) {
			//render points
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			UINT strides[2];
			UINT offsets[2];
			strides[0] = sizeof(VertexPositionColor);
			strides[1] = sizeof(instStruct);
			offsets[0] = 0;
			offsets[1] = 0;
			ID3D11Buffer* bufferPointers[2];
			bufferPointers[0] = this->vertexPlaneBuffer.Get();
			bufferPointers[1] = (*it).GetPointsInstanceBuffer();
			context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

			context->IASetInputLayout(this->pointLayout.Get());
			context->VSSetConstantBuffers1(0, 1, cbTF2DScale.GetAddressOf(), nullptr, nullptr);
			context->VSSetShader(VSPointInst.Get(), nullptr, 0);
			context->PSSetShader(PSPointInst.Get(), nullptr, 0);
			//context->UpdateSubresource1(cbPSPointStuct.Get(), 0, NULL, &PSPointStructInst, 0, 0, 0);
			//context->UpdateSubresource1(cbVSPointStuct.Get(), 0, NULL, &VSPointStructInst, 0, 0, 0);

			int vertexCount = 6;
			context->DrawInstanced(vertexCount, (*it).GetPointsNum(), 0, 0);
		}

		//render lines
		if ((*it).GetLinesNum() > 0) {
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			UINT stride = sizeof(linesDataStruct);
			UINT offset = 0;
			ID3D11Buffer* vertexBuffer;
			vertexBuffer = (*it).GetLinesVertexBuffer();
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

			context->IASetInputLayout(this->linesLayout.Get());
			context->VSSetConstantBuffers1(0, 1, cbTF2DScale.GetAddressOf(), nullptr, nullptr);
			context->VSSetShader(VSLines.Get(), nullptr, 0);
			context->PSSetShader(PSLines.Get(), nullptr, 0);
			context->PSSetConstantBuffers(0, 1, cbPSPointStuct.GetAddressOf());

			context->Draw((*it).lines.size()*2, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}
	deviceResources->EnableDepthTest();
}

void TissueFiguresManager::AddPointToTissue(float x, float y, unsigned int tissueId)
{
	x = ((x / deviceResources->GetLogicalSize().Width) - 0.5f) * 2.0f;
	y = ((y / deviceResources->GetLogicalSize().Height) - 0.5f) * 2.0f;

	for (auto it = this->tissueFigures.begin(); it != tissueFigures.end(); ++it) {
		if (tissueId == (*it).GetTissueId()) {
			(*it).AddPoint(x, y, deviceResources);
			if ((*it).GetPointsNum() == 3) {
				this->UpdateFigCom = true;
			}
		}
	}
}

void TissueFiguresManager::UpdateColors(TissueElement^ sender)
{
	for (auto it = this->tissueFigures.begin(); it != this->tissueFigures.end(); ++it) {
		if ((*it).GetTissueId() == sender->GetId()) { 
			(*it).SetColor(sender->GetColor()); 
			(*it).SetOpacity(sender->GetOpacity());
			this->UpdateFigCom = true;
		}
	}
}



void FillFigure(ID3D11Texture2D* outputTexture, TissueFigure2D* figure,
	std::shared_ptr<DX::DeviceResources> deviceResources)
{
	auto context = deviceResources->GetD3DDeviceContext();
	//map the resource
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(outputTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DirectX::XMFLOAT4* pData = (DirectX::XMFLOAT4*)mappedResource.pData;
	
	//get text desc
	D3D11_TEXTURE2D_DESC desc;
	outputTexture->GetDesc(&desc);
	int width = desc.Width;
	int height = desc.Height;
	
	//find max min y value in points
	int minY = height, maxY = 0;
	for (auto it = figure->points.begin(); it != figure->points.end(); ++it) {
		// [-1,+1] to [0, textureSize]
		float y = (((*it).GetPosition().Y / 2.0f) + 0.5f) * height;
		if (y < minY) minY = y+0.5f;
		if (y > maxY) maxY = y+0.5f;
	}

	//find edges
	struct EdgePixel {
		Windows::Foundation::Point position; // in texture coordinates [0, textSize]
		bool isPoint;
	};

	int ySize = maxY - minY + 1;
	std::vector<Windows::Foundation::Point>* pixels = new std::vector<Windows::Foundation::Point>[ySize];

	for (auto it = figure->lines.begin(); it != figure->lines.end(); ++it) {
		Windows::Foundation::Point startPos = (*it).startPos;
		Windows::Foundation::Point endPos = (*it).endPos;
		// [-1,+1] to [0, textureSize]
		startPos.X = (startPos.X / 2.0f + 0.5f) * width;
		startPos.Y = (startPos.Y / 2.0f + 0.5f) * height;
		endPos.X = (endPos.X / 2.0f + 0.5f) * width;
		endPos.Y = (endPos.Y / 2.0f + 0.5f) * height;

		if (abs(startPos.Y - endPos.Y) >= abs(startPos.X - endPos.X)) {		
			if (startPos.Y > endPos.Y) {
				Windows::Foundation::Point tmp = startPos;
				startPos = endPos;
				endPos = tmp;
			}
			float stepX = (endPos.X - startPos.X) / abs(startPos.Y - endPos.Y + 1);

			for (int i = 0; i <= (int)(endPos.Y - startPos.Y); ++i) {
				pixels[int(startPos.Y - minY + i)].push_back(Windows::Foundation::Point((int)(startPos.X + stepX * i), int(startPos.Y + i)));
			}
		}
		else {			
			if (startPos.X > endPos.X) {
				Windows::Foundation::Point tmp = startPos;
				startPos = endPos;
				endPos = tmp;
			}
			float stepY = (endPos.Y - startPos.Y) / abs(startPos.X - endPos.X + 1);
			for (int i = 0; i <= (int)(endPos.X - startPos.X); ++i) {
				pixels[int(startPos.Y - minY + stepY * i)].push_back(Windows::Foundation::Point(int(startPos.X + i), int(startPos.Y + stepY * i)));
			}
		}
	}


	// sort by x
	for (int i = 0; i < ySize; ++i) {
		for (int j = 0; j < pixels[i].size(); ++j) {
			for (int l = j; l < pixels[i].size(); ++l) {
				if (pixels[i].at(j).X > pixels[i].at(l).X) {
					auto tmp = pixels[i].at(j);
					pixels[i].at(j) = pixels[i].at(l);
					pixels[i].at(l) = tmp;
				}
			}
		}
	}

	//fill
	for (int i = 0; i < ySize; ++i) {
		if (pixels[i].size() > 1){
			for (auto it = pixels[i].begin(); it != pixels[i].end() - 1; ++it) {
				int start = (*it).X;
				int end = (*(it + 1)).X;
				for (int j = start; j <= end; ++j) {
					pData[int(j + width * (*it).Y)].x = figure->color.R / 256.0f;
					pData[int(j + width * (*it).Y)].y = figure->color.G / 256.0f;
					pData[int(j + width * (*it).Y)].z = figure->color.B / 256.0f;
					pData[int(j + width * (*it).Y)].w = figure->opacity;
				}

			}
		}
	}	
	//unmap the resource
	context->Unmap(outputTexture, 0);
	delete[] pixels;
	
}

void Sample3DSceneRenderer::UpdateExposure(float exposure) {
	this->TF2DScaleStruct.exposure = exposure;
}
void Sample3DSceneRenderer::UpdateBrainThresholdValue(float newValue) {
	this->MRIParamStructInst.brainThresholdValue = newValue;
	isUpdateMRIParamBuffer = true;
}
void Sample3DSceneRenderer::UpdateSurfaceThresholdValue(float newValue) {
	this->MRIParamStructInst.surfaceThresholdValue = newValue;
	isUpdateMRIParamBuffer = true;
}

void Sample3DSceneRenderer::SetEegActivityNormalization(float value) {
	this->MRIParamStructInst.eegActivityNormalization = value; 
	this->isUpdateMRIParamBuffer = true; 
}


void Sample3DSceneRenderer::SetEegActivityExp(float value) {
	this->MRIParamStructInst.eegActivityExp = value;
	this->isUpdateMRIParamBuffer = true;
}


void Sample3DSceneRenderer::UpdateMRITextureFromRawDataFloat(float* inputRawData, int xSize, int ySize, int zSize) {
	this->rawTextureData = inputRawData;
	this->xSizeTextureRaw = xSize;
	this->ySizeTextureRaw = ySize;
	this->zSizeTextureRaw = zSize;

	//this->MRITextureReady = true;
	createFromRaw = true;
}

void Sample3DSceneRenderer::UpdateSecondMRITextureFromRawDataFloat(float* inputRawData, int xSize, int ySize, int zSize) {
	this->rawTextureData = inputRawData;
	this->xSizeSecondTextureRaw = xSize;
	this->ySizeSecondTextureRaw = ySize;
	this->zSizeSecondTextureRaw = zSize;

	createFromRaw = true;
	createSecond = true;
	useSecondTexture = true;
}


void Sample3DSceneRenderer::AddToRotationX(float x) {
	this->rotator.x += x;
}

void Sample3DSceneRenderer::AddToRotationY(float y) {
	this->rotator.y += y;
}

void Sample3DSceneRenderer::AddToRotationZ(float z) {
	this->rotator.z += z;
}


void Sample3DSceneRenderer::UpdateElectrode() {
	MRIParamStructInst.mousePosX = this->elMousePosX;
	MRIParamStructInst.mousePosY = this->elMousePosY;
	MRIParamStructInst.UpdateElectrode = idOfActiveElectrode;
}

void Sample3DSceneRenderer::TrackingElectrode() {
	MRIParamStructInst.mousePosX = this->elMousePosX;
	MRIParamStructInst.mousePosY = this->elMousePosY;
	if (idOfActiveElectrode < 0) { 
		MRIParamStructInst.UpdateElectrode = -1; 
	}
	else MRIParamStructInst.UpdateElectrode = 0;
}


void Sample3DSceneRenderer::AddElectrode() {
	
	//Увеличиваем текстуру на +1 размерность и переносим старные данные
	ElectrodeStruct newElectrodeStruct;
	newElectrodeStruct.id = idOfActiveElectrode;
	newElectrodeStruct.texPos = ++this->numElectrodes;
	newElectrodeStruct.value = 0;
	MRIParamStructInst.numElectrodes = this->numElectrodes;
	m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(cbMRIParam.Get(), 0, NULL, &MRIParamStructInst, 0, 0, 0);
	//recreate electrodeTexture
	D3D11_MAPPED_SUBRESOURCE  mappedElectrodePositions;
	if (this->numElectrodes > 1) {
		//read old texture
		this->m_deviceResources->GetD3DDeviceContext()->Map(this->electrodeTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedElectrodePositions);

		//create new texture for electrode data
		CD3D11_TEXTURE1D_DESC texture1DDesc;
		ZeroMemory(&texture1DDesc, sizeof(texture1DDesc));
		texture1DDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		texture1DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		texture1DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texture1DDesc.Width = this->numElectrodes;
		texture1DDesc.MipLevels = 1;
		texture1DDesc.Usage = D3D11_USAGE_DEFAULT;
		texture1DDesc.MiscFlags = 0;
		texture1DDesc.ArraySize = 1;

		delete[] this->electrodeRawTexture;
		this->electrodeRawTexture = new float[this->numElectrodes * 4];
		ZeroMemory(electrodeRawTexture, sizeof(float) * this->numElectrodes * 4);
		for (int i = 0; i < (this->numElectrodes - 1) * 4; ++i) {
			this->electrodeRawTexture[i] = ((float*)(mappedElectrodePositions.pData))[i];
		}
		for (int i = (this->numElectrodes - 1) * 4; i < (this->numElectrodes) * 4; ++i) {
			this->electrodeRawTexture[i] = -10000;
		}
		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = electrodeRawTexture;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		m_deviceResources->GetD3DDevice()->CreateTexture1D(&texture1DDesc, &initData, electrodeTexture.ReleaseAndGetAddressOf());
		m_deviceResources->GetD3DDevice()->CreateUnorderedAccessView(electrodeTexture.Get(), NULL, uavElectrodeTexture.ReleaseAndGetAddressOf());

	}
	
}

void Sample3DSceneRenderer::UpdateElectrodeValues() {

	if (!useSecondTexture) return;
	if (this->electrodeValues.size() != this->numElectrodes || this->electrodeValues.size()==0) return;

	/*for (int i = 0; i < this->numElectrodes-1; ++i) {
		electrodeRawTexture[(i+1)*4+3] = input.at(i);
	}*/

	D3D11_MAPPED_SUBRESOURCE  mappedElectrodePositions;
	this->m_deviceResources->GetD3DDeviceContext()->Map(this->electrodeTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedElectrodePositions);

	delete[] this->electrodeRawTexture;
	this->electrodeRawTexture = new float[this->numElectrodes * 4];
	ZeroMemory(electrodeRawTexture, sizeof(float) * this->numElectrodes * 4);
	for (int i = 0; i < (this->numElectrodes) * 4; ++i) {
		this->electrodeRawTexture[i] = ((float*)(mappedElectrodePositions.pData))[i];
	}

	for (int i = 0; i < this->numElectrodes; ++i) {
		this->electrodeRawTexture[i*4+3] = this->electrodeValues.at(i);
	}

	//delete
	//this->electrodeRawTexture[4] = 0.46;
	//this->electrodeRawTexture[5] = 0.20;
	//this->electrodeRawTexture[6] = 0.64;
	// /delete
	std::vector<float> test;
	for (int i = 0; i < this->numElectrodes*4; ++i) {
		test.push_back(this->electrodeRawTexture[i]);
	}

	CD3D11_TEXTURE1D_DESC texture1DDesc;
	ZeroMemory(&texture1DDesc, sizeof(texture1DDesc));
	texture1DDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	texture1DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texture1DDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texture1DDesc.Width = this->numElectrodes;
	texture1DDesc.MipLevels = 1;
	texture1DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture1DDesc.MiscFlags = 0;
	texture1DDesc.ArraySize = 1;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = electrodeRawTexture;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	m_deviceResources->GetD3DDevice()->CreateTexture1D(&texture1DDesc, &initData, electrodeTexture.ReleaseAndGetAddressOf());
	m_deviceResources->GetD3DDevice()->CreateUnorderedAccessView(electrodeTexture.Get(), NULL, uavElectrodeTexture.ReleaseAndGetAddressOf());

	//m_deviceResources->GetD3DDeviceContext()->UpdateSubresource1(electrodeTexture.Get(), 0, NULL, electrodeRawTexture, 0, 0, 0);

}


void Sample3DSceneRenderer::ChangeElectrodeValues(std::vector<float> input) {
	if (input.size() != this->numElectrodes - 1) return;
	input.insert(input.begin(), 0);
	this->electrodeValues = input;
}

void Sample3DSceneRenderer::SetElectrodeToAdd() {
	isAddElectrode = true;
	++numElectrodesToAdd;
	this->isUpdateMRIParamBuffer = true;
}