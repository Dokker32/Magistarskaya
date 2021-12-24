//
// DirectXPage.xaml.h
// Объявление класса DirectXPage.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "MRIRenderRCTMain.h"
#include "dicom3D.h"
#include "ChartManager.h"

namespace MRIRenderRCT
{
	/// <summary>
	/// Страница, на которой находится класс SwapChainPanel DirectX.
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

	private:
		// Обработчик события низкоуровневой прорисовки XAML.
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// Обработчики событий окна.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// Обработчики события DisplayInformation
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Обработчики других событий.
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		// Отслеживание независимого ввода в фоновом рабочем потоке.
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;
		bool m_coreInputIsReady;
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker2;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput2;
		bool m_coreInputIsReady2;

		// Функции обработки независимого ввода.
		void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		void OnPointerPressedTF2D(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerMovedTF2D(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerReleasedTF2D(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerEnteredTF2D(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerExitedTF2D(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerEnteredMainSwapChain(Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerExitedMainSwapChain(Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void OnPointerPressedMainSwapChain(Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

		// Ресурсы, используемые для прорисовки содержимого DirectX на заднем плане страницы XAML.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> loadedFile;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvLoadedFile;
		std::unique_ptr<MRIRenderRCTMain> m_main; 
		bool m_windowVisible;

		//my
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CreateTransferFunctionWindow(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnOpenFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnOpenSetFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnOpenBrain_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);		
		void btnCreate2DTransferFunction(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnConnectToEEG(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnCreate3DTransferFunction(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);		
		void RealTimeRenderActivate(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void RealTimeRenderDisactivate(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);	
		void SwapChainsPivotSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void TF2DHistPointerWheelChanged(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
		void btnAddTissue(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnAddElectrode(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void TappedTissueElement(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void TappedElectrodeElement(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);		
		void TappedOkButtonColorPicker(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void SliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		void SurfaceThresholdSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		void BrainThresholdSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		void EegActivityNormalizationSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		void EegActivityExpSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		void keyRotateMRIPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ e);
		void EegAverageValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e);
		
		

		float pointerPrX;
		float pointerPrY;
		//DicomSlice3D* dicomSlice;
		DicomSet3D* dicomSet;

		//connecting
		Windows::Networking::Sockets::StreamSocket^ socket;
		ChartList^ chartList;
		ChartManager* chartManager;
	};
}

