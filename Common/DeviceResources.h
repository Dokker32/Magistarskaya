#pragma once
#include <string> 
#include "helper.h"




namespace DX
{
	

	struct SwapChainPanelWarp {
		Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel;
		std::string name;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	d3dDepthStencilView;
		bool isLoaded = false;

		SwapChainPanelWarp( std::string name) :  name(name){};
	};

	class SwapChainWarpHandler {
		std::vector<std::shared_ptr<SwapChainPanelWarp>> objects;
		
	public:
		std :: shared_ptr<SwapChainPanelWarp> getWarp(std::string name);

	};
	// Предоставляет интерфейс, позволяющий владеющему DeviceResources приложению получать уведомления о потере или создании устройства.
	interface IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	// Управляет всеми ресурсами устройств DirectX.
	class DeviceResources
	{
	public:
		DeviceResources();
		void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
		void SetLogicalSize(Windows::Foundation::Size logicalSize);
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
		void SetDpi(float dpi);
		void SetCompositionScale(float compositionScaleX, float compositionScaleY);
		void ValidateDevice();
		void HandleDeviceLost();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
		void Trim();
		void Present(CurrentSwapChain currentSwapChain);

		// Размер целевого объекта отрисовки в пикселях.
		Windows::Foundation::Size	GetOutputSize() const { return m_outputSize; }

		// Размер целевого объекта отрисовки в аппаратно-независимых пикселях (DIP).
		Windows::Foundation::Size	GetLogicalSize() const { return m_logicalSize; }
		float						GetDpi() const { return m_effectiveDpi; }

		// Методы доступа к D3D.
		ID3D11Device3* GetD3DDevice() const { return m_d3dDevice.Get(); }
		ID3D11DeviceContext3* GetD3DDeviceContext() const { return m_d3dContext.Get(); }
		IDXGISwapChain3* GetSwapChain() const { return m_swapChain.Get(); }
		D3D_FEATURE_LEVEL			GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
		ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return m_d3dRenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_d3dDepthStencilView.Get(); }
		D3D11_VIEWPORT				GetScreenViewport() const { return m_screenViewport; }
		DirectX::XMFLOAT4X4			GetOrientationTransform3D() const { return m_orientationTransform3D; }
		ID3D11BlendState* GetBlendState() const { return blendState.Get(); }

		// Методы доступа к D2D.
		ID2D1Factory3* GetD2DFactory() const { return m_d2dFactory.Get(); }
		ID2D1Device2* GetD2DDevice() const { return m_d2dDevice.Get(); }
		ID2D1DeviceContext2* GetD2DDeviceContext() const { return m_d2dContext.Get(); }
		ID2D1Bitmap1* GetD2DTargetBitmap() const { return m_d2dTargetBitmap.Get(); }
		IDWriteFactory3* GetDWriteFactory() const { return m_dwriteFactory.Get(); }
		IWICImagingFactory2* GetWicImagingFactory() const { return m_wicFactory.Get(); }
		D2D1::Matrix3x2F			GetOrientationTransform2D() const { return m_orientationTransform2D; }

		//my
		void SetMainRenderTarget() const { m_d3dContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get()); };
		void setBackRasterizerState() const { m_d3dContext->RSSetState(backRasterState.Get()); };
		void setFrontRasterizerState() const { m_d3dContext->RSSetState(frontRasterState.Get()); };
		Windows::Foundation::Size getRenderTargetSize() { return m_d3dRenderTargetSize; }
		void CatchSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel, std::string name);
		SwapChainWarpHandler* getSwapChainWarpHandler();
		void DisableDepthTest() const { m_d3dContext->OMSetDepthStencilState(depthStencilState.Get(), 1); };
		void EnableDepthTest() const { m_d3dContext->OMSetDepthStencilState(NULL, 1); };
	

	private:
		void CreateDeviceIndependentResources();
		void CreateDeviceResources();
		void CreateWindowSizeDependentResources();
		void UpdateRenderTargetSize();
		DXGI_MODE_ROTATION ComputeDisplayRotation();

		//my
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	frontRasterState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	backRasterState;
		// Объекты Direct3D.
		Microsoft::WRL::ComPtr<ID3D11Device3>			m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext3>	m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11BlendState>		blendState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

		// Объекты прорисовки Direct3D. Требуются для трехмерной прорисовки.
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
		D3D11_VIEWPORT									m_screenViewport;

		// Компоненты рисования Direct2D.
		Microsoft::WRL::ComPtr<ID2D1Factory3>		m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device2>		m_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext2>	m_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>		m_d2dTargetBitmap;

		// Компоненты рисования DirectWrite.
		Microsoft::WRL::ComPtr<IDWriteFactory3>		m_dwriteFactory;
		Microsoft::WRL::ComPtr<IWICImagingFactory2>	m_wicFactory;

		// Кэшированная ссылка на панель XAML.
		Windows::UI::Xaml::Controls::SwapChainPanel^ m_swapChainPanel;

		// Кэшированные свойства устройств.
		D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
		Windows::Foundation::Size						m_d3dRenderTargetSize;
		Windows::Foundation::Size						m_outputSize;
		Windows::Foundation::Size						m_logicalSize;
		Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
		float											m_dpi;
		float											m_compositionScaleX;
		float											m_compositionScaleY;

		// Переменные, которые учитывают, поддерживает ли приложение экраны высокого разрешения или нет.
		float											m_effectiveDpi;
		float											m_effectiveCompositionScaleX;
		float											m_effectiveCompositionScaleY;

		// Преобразования, используемые для ориентации экрана.
		D2D1::Matrix3x2F	m_orientationTransform2D;
		DirectX::XMFLOAT4X4	m_orientationTransform3D;

		// Объект IDeviceNotify можно сохранять напрямую, так как он владеет DeviceResources.
		IDeviceNotify* m_deviceNotify;

		//my
		// Объекты прорисовки Direct3D. Требуются для трехмерной прорисовки.
		SwapChainWarpHandler swapChainWarpHandler;
	};
}