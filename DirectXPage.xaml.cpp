//
// DirectXPage.xaml.cpp
// Реализация класса DirectXPage.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include "BlankPage.xaml.h"
#include "nifti/laynii_lib.h"



using namespace MRIRenderRCT;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Popups;
using namespace concurrency;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr),
	m_coreInputIsReady(false)
{
	InitializeComponent();

	// Регистрация обработчиков событий для жизненного цикла страницы.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::keyRotateMRIPressed);
	

	// На этом этапе у нас есть доступ к устройству. 
	// Мы можем создавать ресурсы, зависящие от устройства.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);
	
	// Регистрация SwapChainPanel для получения независимых событий ввода указателя
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// Объект CoreIndependentInputSource вызовет события указателя для указанных типов устройств в том потоке, в котором он создан.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Регистрация для получения событий указателя, которые создаются в фоновом потоке.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressedMainSwapChain);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);
		m_coreInput->PointerEntered += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerEnteredMainSwapChain);
		m_coreInput->PointerExited += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerExitedMainSwapChain);

		// Начало обработки входных сообщений по мере их доставки.
		m_coreInputIsReady = true;
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);	
	});

	// Выполнение задачи в выделенном высокоприоритетном фоновом потоке.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	m_main = std::unique_ptr<MRIRenderRCTMain>(new MRIRenderRCTMain(m_deviceResources));
	m_main->StartRenderLoop();

	//set test tissue element
	TissueElement^ newTissueElement = ref new TissueElement();
	m_main->AddTissue(newTissueElement);
	newTissueElement->Margin = Windows::UI::Xaml::Thickness(2);
	newTissueElement->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedTissueElement);
	//TissuesStackPanel->Children->Append(newTissueElement);
}

DirectXPage::~DirectXPage()
{
	// Остановка прорисовки и обработки событий при уничтожении объекта.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Сохраняет текущее состояние приложения для событий приостановки действия и завершения.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Остановка прорисовки при приостановке действия приложения.
	m_main->StopRenderLoop();

	// Поместите сюда код для сохранения состояния приложения.
}

// Загружает текущее состояние приложения для событий возобновления.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Поместите сюда код для загрузки состояния приложения.

	// Запуск прорисовки при возобновлении приложения.
	m_main->StartRenderLoop();
}

// Обработчики событий окна.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// Обработчики события DisplayInformation

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Примечание. Значение LogicalDpi, полученное здесь, может не соответствовать фактическому DPI приложения,
	// если его масштаб изменяется для устройств с экраном высокого разрешения. После установки DPI в DeviceResources,
	// всегда следует получать его с помощью метода GetDpi.
	// См. DeviceResources.cpp для получения дополнительных сведений.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Вызывается при нажатии кнопки на панели приложения.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Используйте панель приложения, если она нужна. Разработайте панель приложения, 
	// затем заполните обработчики событий (как этот).
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// Нажмите на указатель и начните отслеживание его перемещений.
	m_coreInput->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::SizeAll, 0);	
	m_main->StartTracking();	
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// Обновление кода отслеживания указателя.
	if (m_main->IsTracking())
	{
		m_main->TrackingUpdate(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
	}
	if (m_main->activeElectrodeElement != nullptr) {
		m_main->UpdateElectrodeMousePos(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
	}
}


void DirectXPage::OnPointerEnteredMainSwapChain(Object^ sender, PointerEventArgs^ e)
{
	m_main->StartTrackingElectrode();
}

void DirectXPage::OnPointerExitedMainSwapChain(Object^ sender, PointerEventArgs^ e)
{
	m_main->FinishTrackingElectrode();
}

void DirectXPage::OnPointerPressedMainSwapChain(Object^ sender, PointerEventArgs^ e)
{
	m_main->UpdateElectrode();
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Остановка отслеживания движений указателя при освобождении указателя.
	m_main->StopTracking();
	m_coreInput->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Arrow, 0);	
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//MessageDialog^ msg = ref new MessageDialog("No internet connection has been found.");
	//msg->ShowAsync();
	
	auto m = m_deviceResources;
	create_async([m] {
		Microsoft::WRL::ComPtr<ID3D11Texture3D> texture;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> textureConv;
		DX::LoadTestDat(&texture, m->GetD3DDevice());
		//deviceResources->GetD3DDevice()->CreateShaderResourceView(ttt.Get(), NULL, srvTexture3D.ReleaseAndGetAddressOf());
		CD3D11_TEXTURE3D_DESC textureDesc;
		texture.Get()->GetDesc(&textureDesc);
		int width = textureDesc.Width;
		int height = textureDesc.Height;
		int length = textureDesc.Depth;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture3D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTexture3DConv;
		m->GetD3DDevice()->CreateShaderResourceView(texture.Get(), NULL, srvTexture3D.ReleaseAndGetAddressOf());
		m->GetD3DDeviceContext()->CSSetShaderResources(0, 1, srvTexture3D.GetAddressOf());
		ApplyConvolution(texture.Get(), textureConv.ReleaseAndGetAddressOf(),
			m->GetD3DDevice(), m->GetD3DDeviceContext(), 7, 3);
		m->GetD3DDevice()->CreateShaderResourceView(textureConv.Get(), NULL, srvTexture3DConv.ReleaseAndGetAddressOf());
		m->GetD3DDeviceContext()->CSSetShaderResources(1, 1, srvTexture3DConv.GetAddressOf());
	});


}

void DirectXPage::CreateTransferFunctionWindow(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	auto c = Concurrency::create_task(Windows::UI::WindowManagement::AppWindow::TryCreateAsync());
	auto p = c.then([](Windows::UI::WindowManagement::AppWindow^ aw) {
		Windows::UI::Xaml::Controls::Frame^ f = ref new Windows::UI::Xaml::Controls::Frame();
		f->Navigate(MRIRenderRCT::BlankPage::typeid);
		Windows::UI::Xaml::Hosting::ElementCompositionPreview::SetAppWindowContent(aw, f);
		aw->TryShowAsync();
	});
}

void DirectXPage::btnOpenFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	auto picker = ref new Windows::Storage::Pickers::FileOpenPicker();
	picker->ViewMode = Windows::Storage::Pickers::PickerViewMode::Thumbnail;
	picker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::Desktop;
	picker->FileTypeFilter->Append(".dat");
	picker->FileTypeFilter->Append(".dcm");

	auto device = m_deviceResources->GetD3DDevice();
	auto outTexture = this->loadedFile.ReleaseAndGetAddressOf();

	auto findFileTask = Concurrency::create_task(picker->PickSingleFileAsync());	
	auto openFileTask = findFileTask.then([this](Windows::Storage::StorageFile^ file) {
		if (file != nullptr) {
			if (file->FileType == ".dat") {
				auto output = Windows::Storage::FileIO::ReadBufferAsync(file);
				return output;
			}
			else if (file->FileType == ".dcm") {

				Platform::String^ fileName = file->Name;
				std::wstring wstrN(fileName->Begin());
				std::string strN(wstrN.begin(), wstrN.end());

				float* rawData = nullptr;
				int* xDim = nullptr;
				int* yDim = nullptr;
				int* zDim = nullptr;

				Dicom3D dicom3D(strN);
				dicom3D.GetRawData(&rawData, &xDim, &yDim, &zDim);

				this->m_main->m_sceneRenderer->UpdateMRITextureFromRawDataFloat(rawData, *xDim,
					*yDim, *zDim);

				this->m_main->m_sceneRenderer->UpdateMRITextureSRV();
				this->m_main->m_sceneRenderer->MRITextureSetSizeParameters(*yDim, *xDim, *zDim);
				this->m_main->m_sceneRenderer->MRITextureReady = true;
				this->m_main->m_sceneRenderer->DoRender();

				this->m_main->isSurfaceTextureLoaded = true;
				this->SurfaceThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
				this->SurfaceThresholdSlider->Maximum = dicom3D.GetMaxValue();
				this->OpenBrainButton->IsEnabled = true;
				cancel_current_task();
			}
			else {
				//error
			}
		}
		else {
			cancel_current_task();
		}
	});
	auto readFileTask = openFileTask.then([this, outTexture, device](Windows::Storage::Streams::IBuffer^ buff) {
		auto dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buff);
		Platform::Array<byte>^ data = ref new Platform::Array<byte>(buff->Length);
		dataReader->ReadBytes(data);

		uint16_t xDim = *((uint16_t*)&(data[0]));
		uint16_t yDim = *((uint16_t*)&(data[2]));
		uint16_t zDim = *((uint16_t*)&(data[4]));

		unsigned int channels = 1;
		unsigned int bytesPerChannel = sizeof(float);
		unsigned int textureNumValues = xDim * yDim * zDim * channels;

		float* pTextureData = new float[channels * xDim * yDim * zDim];

		for (int i = 0; i < (channels * xDim * yDim * zDim); ++i) {
			uint16_t value = *((uint16_t*)&data[6 + i * 2]);
			pTextureData[i] = value;
		}	

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

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		HRESULT HR = device->CreateTexture3D(&textureDesc, &initData, outTexture);

		if (m_coreInputIsReady) {
			m_coreInput->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
				ref new Windows::UI::Core::DispatchedHandler([this, outTexture, xDim, yDim, zDim]() {				
				auto pointer = m_main->m_sceneRenderer->getMRITexturePointer();
				*pointer = *outTexture;
				m_main->m_sceneRenderer->UpdateMRITextureSRV();
				m_main->m_sceneRenderer->MRITextureSetSizeParameters(yDim,xDim,zDim);
				m_main->m_sceneRenderer->MRITextureReady = true;
				m_main->m_sceneRenderer->DoRender();
			}));
			this->m_main->isSurfaceTextureLoaded = true;
			this->SurfaceThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
			
		}
		else {
			MessageDialog^ msg = ref new MessageDialog("m_CoreInput is not ready");
			msg->ShowAsync();
		}
			
	
	});

}


void DirectXPage::btnOpenBrain_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {

	auto picker = ref new Windows::Storage::Pickers::FileOpenPicker();
	picker->ViewMode = Windows::Storage::Pickers::PickerViewMode::Thumbnail;
	picker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::Desktop;
	picker->FileTypeFilter->Append(".nii");

	auto device = m_deviceResources->GetD3DDevice();
	auto outTexture = this->loadedFile.ReleaseAndGetAddressOf();

	auto findFileTask = Concurrency::create_task(picker->PickSingleFileAsync());
	auto openFileTask = findFileTask.then([this](Windows::Storage::StorageFile^ file) {
		if (file != nullptr) {
			Platform::String^ filePath = file->Path;
			std::wstring wstrFP(filePath->Begin());
			std::string strFP(wstrFP.begin(), wstrFP.end());
			nifti_image* nim = NULL;
			nim = nifti_image_read("output1.nii", 1);

			float* rawData = nullptr;
			if (nim->datatype == NIFTI_TYPE_INT16) {
				float* tmpData = new float[nim->nvox];
				for (int i = 0; i < nim->nvox; ++i) {
					tmpData[i] = ((int16_t*)nim->data)[i];
				}
				rawData = tmpData;
			}

			if (nim->datatype == NIFTI_TYPE_FLOAT32) {
				rawData = (float*)nim->data;
			}		

			//reflect y axis
			for (int x = 0; x < nim->nx; ++x) {
				for (int y = 0; y < int(nim->ny/2.0); ++y) {
					for (int z = 0; z < nim->nz; ++z) {
						float tmpValue = rawData[x + y*nim->nx + z*nim->nx*nim->ny];
						rawData[x + y * nim->nx + z * nim->nx * nim->ny] = rawData[x + (nim->ny - 1 - y) * nim->nx + z * nim->nx * nim->ny];
						rawData[x + (nim->ny - 1 - y) * nim->nx + z * nim->nx * nim->ny] = tmpValue;
					}
				}
			}

			//			
			unsigned int maxValue = 1;
			for (int i = 0; i < nim->nvox; ++i) {
				if (rawData[i] > maxValue) maxValue = rawData[i];
				//if (rawData[i]<500000) rawData[i] = 0;
			}
			
			this->m_main->m_sceneRenderer->UpdateSecondMRITextureFromRawDataFloat(rawData, nim->nx,
				nim->ny, nim->nz);
			this->m_main->m_sceneRenderer->UpdateMRITextureSRV();
			int xDim = nim->nx;
			int yDim = nim->ny;
			int zDim = nim->nz;
			this->m_main->m_sceneRenderer->MRITextureSetSizeParameters(yDim, xDim, zDim);
			this->m_main->m_sceneRenderer->MRITextureReady = true;
			this->m_main->m_sceneRenderer->DoRender();	

			this->m_main->isBrainTextureLoaded = true;
			this->BrainThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
			this->BrainThresholdSlider->TickFrequency = maxValue / 1000.0;
			this->BrainThresholdSlider->Maximum = maxValue;
			this->EegActivityNormalization->Visibility = Windows::UI::Xaml::Visibility::Visible;
			this->EegActivityNormalization->Minimum = 1;
			this->EegActivityExp->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}
		else {
			cancel_current_task();
		}
	});

}


void DirectXPage::btnOpenSetFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	auto picker = ref new Windows::Storage::Pickers::FileOpenPicker();
	picker->ViewMode = Windows::Storage::Pickers::PickerViewMode::Thumbnail;
	picker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::Desktop;
	//picker->FileTypeFilter->Append(".dat");
	picker->FileTypeFilter->Append(".dcm");
	
	auto device = m_deviceResources->GetD3DDevice();
	auto outTexture = this->loadedFile.ReleaseAndGetAddressOf();

	auto findFileTask = Concurrency::create_task(picker->PickMultipleFilesAsync());
	auto openAndReadFileTask = findFileTask.then([this, outTexture](Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile^>^ files) {
		if (files->Size > 0) {
			std::vector<std::vector<byte>*>* filesVec = new std::vector<std::vector<byte>*>();
			filesVec->resize(files->Size);
			for (int i = 0; i < files->Size; ++i) {
				filesVec->at(i) = new std::vector<byte>();
			}
			//std::vector<byte>* dataO = new std::vector<byte>();	
			//num of bytes
			uint32_t bytesNum = 0;
			//load data in dataL
			std::vector<task<void>> tasks = {};

			for (int i = 0; i < files->Size; ++i) {
				auto out = Windows::Storage::FileIO::ReadBufferAsync(files->GetAt(i));

				tasks.push_back(Concurrency::create_task(out).then([this, filesVec, i](Windows::Storage::Streams::IBuffer^ buff) {
					auto dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buff);
					Platform::Array<byte>^ data = ref new Platform::Array<byte>(buff->Length);
					dataReader->ReadBytes(data);
					std::vector<byte> tmpStdVector(begin(data), end(data));
					auto tmpV = ref new Platform::Collections::Vector<byte>(data);
					
					filesVec->at(i)->insert(filesVec->at(i)->end(), tmpStdVector.begin(), tmpStdVector.end());
				}, task_continuation_context::use_default()));

				//Windows::Storage::Streams::IBuffer^ buff = out->GetResults();
				//auto dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buff);
				//Platform::Array<byte>^ data = ref new Platform::Array<byte>(buff->Length);
				//dataReader->ReadBytes(data);
				//dataL[i] = data;
				//bytesNum += dataL[i]->Length;

			}
			//dataO->push_back(1);
			//create output file
			/*Windows::Storage::StorageFolder^ storageFolder =
				Windows::Storage::ApplicationData::Current->LocalFolder;
			auto createFileTask = Concurrency::when_all(tasks.begin(), tasks.end()).then([storageFolder]() {
				return storageFolder->CreateFileAsync("sample.txt",
					Windows::Storage::CreationCollisionOption::ReplaceExisting);
			});*/
			//write into output file
			/*auto writeDataTask = createFileTask.then([this, filesVec](Windows::Storage::StorageFile^ storageFile) {
				std::vector<byte>* tmpBytesVec = new std::vector<byte>();
				for (int i = 1; i < filesVec->size(); ++i) {
					tmpBytesVec->insert(tmpBytesVec->end(), filesVec->at(i)->begin(), filesVec->at(i)->end());
				}

				Platform::Array<byte>^ byteDataO = ref new Platform::Array<byte>(tmpBytesVec->data(), tmpBytesVec->size());
				Windows::Storage::FileIO::WriteBytesAsync(storageFile, byteDataO);

				////auto lTask = create_task(Windows::Storage::FileIO::WriteBytesAsync(storageFile, byteDataO));
				for (int i = 1; i < filesVec->size(); ++i) {
					byteDataO = ref new Platform::Array<byte>(filesVec->at(i)->data(), filesVec->at(i)->size());
					//convert bytes to platform::string cause no AppendBytesAsync - only AppendTextAssync
					lTask = lTask.then([storageFile, byteDataO]() {
						Windows::Storage::FileIO::WriteBytesAsync(storageFile, byteDataO);
					});
				}	////
			});*/
			Concurrency::when_all(tasks.begin(), tasks.end()).then([this, filesVec, outTexture] {
				this->dicomSet = new DicomSet3D(filesVec);
				dicomSet->CreateID3D11ShaderResourceView(this->m_deviceResources,
					this->loadedFile.GetAddressOf(), this->srvLoadedFile.GetAddressOf(), 10);			
					
				float* rawData = this->dicomSet->GetTextureFloatData();
				this->m_main->m_sceneRenderer->UpdateMRITextureFromRawDataFloat(rawData, this->dicomSet->GetTexDesc().columns,
					this->dicomSet->GetTexDesc().rows, this->dicomSet->GetTexDesc().slices);
				this->m_main->m_sceneRenderer->UpdateMRITextureSRV();
				int xDim = this->dicomSet->GetTexDesc().columns;
				int yDim = this->dicomSet->GetTexDesc().rows;
				int zDim = this->dicomSet->GetTexDesc().slices;
				this->m_main->m_sceneRenderer->MRITextureSetSizeParameters(yDim, xDim, zDim);
				this->m_main->m_sceneRenderer->MRITextureReady = true;
				this->m_main->m_sceneRenderer->DoRender();

				this->m_main->isSurfaceTextureLoaded = true;
				this->SurfaceThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
				/*int xDim = this->dicomSet->GetTexDesc().columns;
				int yDim = this->dicomSet->GetTexDesc().rows;
				int zDim = this->dicomSet->GetTexDesc().slices;
				auto pointer = m_main->m_sceneRenderer->getMRITexturePointer();
				*pointer = *outTexture;
				m_main->m_sceneRenderer->UpdateMRITextureSRV();
				m_main->m_sceneRenderer->MRITextureSetSizeParameters(yDim, xDim, zDim);
				m_main->m_sceneRenderer->MRITextureReady = true;
				m_main->m_sceneRenderer->DoRender();*/		

				/*std::vector<DicomSlice3D*>* dicomSet3D = new std::vector<DicomSlice3D*>();
				dicomSet3D->resize(filesVec->size());
				for (int i = 0; i < filesVec->size(); ++i) {
					dicomSet3D->at(i) = new DicomSlice3D(filesVec->at(i));
					dicomSet3D->at(i)->ReadDesc(filesVec->size());
				}
				//check for uniform data
				for (int i = 1; i < filesVec->size(); ++i) {
					if (dicomSet3D->at(0)->GetTexDesc().columns != dicomSet3D->at(i)->GetTexDesc().columns) throw std::invalid_argument("different num of columns");
					if (dicomSet3D->at(0)->GetTexDesc().rows != dicomSet3D->at(i)->GetTexDesc().rows) throw std::invalid_argument("different num of rows");
					if (dicomSet3D->at(0)->GetTexDesc().slices != dicomSet3D->at(i)->GetTexDesc().slices) throw std::invalid_argument("different num of slices");
					if (dicomSet3D->at(0)->GetTexDesc().bitsAllocated != dicomSet3D->at(i)->GetTexDesc().bitsAllocated) throw std::invalid_argument("different num of bitsAllocated");
					if (dicomSet3D->at(0)->GetTexDesc().bitsActually != dicomSet3D->at(i)->GetTexDesc().bitsActually) throw std::invalid_argument("different num of bitsActually");
					if (dicomSet3D->at(0)->GetTexDesc().bitsHight != dicomSet3D->at(i)->GetTexDesc().bitsHight) throw std::invalid_argument("different num of bitsHight");
					if (dicomSet3D->at(0)->GetTexDesc().channels != dicomSet3D->at(i)->GetTexDesc().channels) throw std::invalid_argument("different num of channels");
				}*/
			});			

			/*Platform::String^ st = storageFolder->Path;
			std::wstring fooW(st->Begin());
			std::string fooA(fooW.begin(), fooW.end());
			int a = filesVec->size();
			int b = 3;
			int c = a + b;*/
			
		}
		else {
			cancel_current_task();
		}
	});

	
	/*auto readFileTask = openAndReadFileTask.then([this, outTexture, device](Windows::Storage::Streams::IBuffer^ buff) {
		auto dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buff);
		Platform::Array<byte>^ data = ref new Platform::Array<byte>(buff->Length);
		dataReader->ReadBytes(data);

		uint16_t xDim = *((uint16_t*)&(data[0]));
		uint16_t yDim = *((uint16_t*)&(data[2]));
		uint16_t zDim = *((uint16_t*)&(data[4]));

		unsigned int channels = 1;
		unsigned int bytesPerChannel = sizeof(float);
		unsigned int textureNumValues = xDim * yDim * zDim * channels;

		float* pTextureData = new float[channels * xDim * yDim * zDim];

		for (int i = 0; i < (channels * xDim * yDim * zDim); ++i) {
			uint16_t value = *((uint16_t*)&data[6 + i * 2]);
			pTextureData[i] = value;
		}

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

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		HRESULT HR = device->CreateTexture3D(&textureDesc, &initData, outTexture);

		if (m_coreInputIsReady) {
			m_coreInput->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
				ref new Windows::UI::Core::DispatchedHandler([this, outTexture, xDim, yDim, zDim]() {
				auto pointer = m_main->m_sceneRenderer->getMRITexturePointer();
				*pointer = *outTexture;
				m_main->m_sceneRenderer->UpdateMRITextureSRV();
				m_main->m_sceneRenderer->MRITextureSetSizeParameters(yDim, xDim, zDim);
				m_main->m_sceneRenderer->MRITextureReady = true;
				m_main->m_sceneRenderer->DoRender();
			}));
		}
		else {
			MessageDialog^ msg = ref new MessageDialog("m_CoreInput is not ready");
			msg->ShowAsync();
		}


	});*/

}	

void DirectXPage::RealTimeRenderActivate(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	if (m_coreInputIsReady) {
		m_coreInput->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([this]() {
			m_main->m_sceneRenderer->setRealTimeRender();
		}));
	}
	else {
		((CheckBox^)sender)->IsChecked = false;
	}
}

void DirectXPage::RealTimeRenderDisactivate(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	if (m_coreInputIsReady) {
		m_coreInput->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([this]() {
			m_main->m_sceneRenderer->unsetRealTimeRender();
		}));
	}
	else {
		((CheckBox^)sender)->IsChecked = true;
	}
}

void DirectXPage::btnCreate2DTransferFunction(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	bool PivotIsAlreadyCreated = false;
	for (int i = 0; i < SwapChainsPivot->Items->Size; ++i) {
		if (((PivotItem^)(SwapChainsPivot->Items->GetAt(i)))->Name == "PivotItem2DTransferFunction") {
			PivotIsAlreadyCreated = true;
		}
	}
	if (!PivotIsAlreadyCreated) {
		PivotItem^ newPivotItem = ref new PivotItem();
		SwapChainPanel^ TransferFunction2DSwapChainPanel = ref new SwapChainPanel();
		TransferFunction2DSwapChainPanel->Name = "TransferFunction2DSwapChainPanel";
		newPivotItem->Content = TransferFunction2DSwapChainPanel;
		newPivotItem->Name = "PivotItem2DTransferFunction";
		newPivotItem->Header = "2D Transfer Function";
		SwapChainsPivot->Items->Append(newPivotItem);

		m_deviceResources->CatchSwapChainPanel(TransferFunction2DSwapChainPanel, "SwapChain2DTransferFunction");

		// Регистрация SwapChainPanel для получения независимых событий ввода указателя
		auto workItemHandler = ref new WorkItemHandler([this, TransferFunction2DSwapChainPanel](IAsyncAction^)
		{
			// Объект CoreIndependentInputSource вызовет события указателя для указанных типов устройств в том потоке, в котором он создан.
			m_coreInput2 = TransferFunction2DSwapChainPanel->CreateCoreIndependentInputSource(
				Windows::UI::Core::CoreInputDeviceTypes::Mouse |
				Windows::UI::Core::CoreInputDeviceTypes::Touch |
				Windows::UI::Core::CoreInputDeviceTypes::Pen
			);
			// Регистрация для получения событий указателя, которые создаются в фоновом потоке.
			m_coreInput2->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Arrow, 0);			
			m_coreInput2->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressedTF2D);
			m_coreInput2->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMovedTF2D);
			m_coreInput2->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleasedTF2D);
			m_coreInput2->PointerWheelChanged += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::TF2DHistPointerWheelChanged);
			m_coreInput2->PointerEntered += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerEnteredTF2D);
			m_coreInput2->PointerExited += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerExitedTF2D);

			// Начало обработки входных сообщений по мере их доставки.
			m_coreInputIsReady2 = true;
			m_coreInput2->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
		});

		m_inputLoopWorker2 = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);


		m_main->m_sceneRenderer->Create2DTransferFunctionResources();

	}
	else {
		//TODO. RECREATE HIST
	}
}

void DirectXPage::btnConnectToEEG(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	bool PivotIsAlreadyCreated = false;
	for (int i = 0; i < SwapChainsPivot->Items->Size; ++i) {
		if (((PivotItem^)(SwapChainsPivot->Items->GetAt(i)))->Name == "PivotItemEEGChart") {
			PivotIsAlreadyCreated = true;
		}
	}
	if (!PivotIsAlreadyCreated) {
		PivotItem^ newPivotItem = ref new PivotItem();
		newPivotItem->Name = "PivotItemEEGChart";
		newPivotItem->Header = "EEG Chart";
		SwapChainsPivot->Items->Append(newPivotItem);
		ChartList^ newChartList = ref new ChartList();
		this->chartManager = new ChartManager(newChartList);
		this->chartList = newChartList;
		/*newChartList->AddChart(L"AA");
		newChartList->AddChart(L"AB");
		newChartList->AddChart(L"AC");
		newChartList->AddChart(L"AD");
		newChartList->AddChart(L"AE");
		newChartList->AddChart(L"AF");
		newChartList->AddChart(L"AG");*/

		newPivotItem->Content = newChartList;
		//test
		/*
		for (int i = 0; i < 29; ++i) {
			Platform::Collections::Vector<float>^ vec = ref new Platform::Collections::Vector<float>();
			for (int j = 0; j < 7; ++j){				
				vec->Append(std::rand() - RAND_MAX / 2);				
			}	
			newChartList->AddPointsToAllCharts(vec);
		}	

		newChartList->Update();*/
		
		//connect by socket etc

		this->socket = ref new Windows::Networking::Sockets::StreamSocket();
		auto hostName = ref new Windows::Networking::HostName(L"169.254.104.244");
		Platform::String^ port = ref new Platform::String(L"120");

		//-test
		Concurrency::create_task(this->socket->ConnectAsync(hostName, port)).then([this](task<void> t) {
			try
			{
				t.get();
				//start study
				//write data
				auto dataWriter = ref new Windows::Storage::Streams::DataWriter(this->socket->OutputStream);
				/*Platform::Array<byte>^ bytes = ref new Platform::Array<byte>(2);
				bytes[0] = 0x01;
				bytes[1] = 0x70;*/

				//затычка\костыль
				Platform::Array<byte>^ bytes = ref new Platform::Array<byte>(8);
				bytes[0] = 0x04;
				bytes[1] = 0x00;
				bytes[2] = 0x00;
				bytes[3] = 0x00;
				bytes[4] = 0x01;
				bytes[5] = 0x00;
				bytes[6] = 0x00;
				bytes[7] = 0x00;
				dataWriter->WriteBytes(bytes);

				Concurrency::create_task(dataWriter->StoreAsync()).then(
					[=](Concurrency::task<unsigned int>)
				{
					//try to read data
					auto dataReader = ref new Windows::Storage::Streams::DataReader(this->socket->InputStream);
					dataReader->ByteOrder= Windows::Storage::Streams::ByteOrder::LittleEndian;
					//костыль
					//numBytesInHeader = sizeof(CString);
					Concurrency::create_task(dataReader->LoadAsync(sizeof(unsigned int))).then(
						[=]	(unsigned int bytesLoaded) 
					{

						uint32_t packSize = dataReader->ReadUInt32();
						//-----
						Concurrency::create_task(dataReader->LoadAsync(packSize)).then(
							[=](unsigned int bytesLoaded)
						{
							//read pack code
							Platform::Array<byte>^ packCode = ref new Platform::Array<byte>(4);
							dataReader->ReadBytes(packCode);
							//read patient name
							uint32_t numChars = dataReader->ReadUInt32();
							Platform::Array<byte>^ patientName = ref new Platform::Array<byte>(numChars);
							dataReader->ReadBytes(patientName);

							//read others
							uint32_t numChannels = dataReader->ReadUInt32();
							std::vector<string>* channelNames = new std::vector<string>();
							for (int i = 0; i < numChannels; ++i) {
								uint32_t numChars = dataReader->ReadUInt32();
								Platform::Array<byte>^ channelName = ref new Platform::Array<byte>(numChars);
								dataReader->ReadBytes(channelName);
								
								std::string strChannelName(channelName->begin(), channelName->end());
								channelNames->push_back(strChannelName);

								std::wstring w_str = std::wstring(strChannelName.begin(), strChannelName.end());
								const wchar_t* w_chars = w_str.c_str();
								Platform::String^ PlatformStrChannelName = ref new Platform::String(w_chars);
								
								ElectrodeElement^ newElectrodeElement = ref new ElectrodeElement(ElectrodeStackPanel->Children->Size);
								newElectrodeElement->SetName(PlatformStrChannelName);
								if (true){
									//critical_section::scoped_lock lock(m_main->GetElectrodeCriticalSection());
									++m_main->electrodesToAdd;
									m_main->AddElectrode(newElectrodeElement);
								}
								newElectrodeElement->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedElectrodeElement);
								newElectrodeElement->Margin = Windows::UI::Xaml::Thickness(2);
								ElectrodeStackPanel->Children->Append(newElectrodeElement);

								chartList->AddChart(PlatformStrChannelName);
							}
							numChars = dataReader->ReadUInt32();
							Platform::Array<byte>^ SchemeReconstraction = ref new Platform::Array<byte>(numChars);
							dataReader->ReadBytes(SchemeReconstraction);

							double Weight = dataReader->ReadDouble();
							double Frequency = dataReader->ReadDouble();

							//читаем сами данные с датчиков
							auto workItemHandler = ref new WorkItemHandler([this, dataReader](IAsyncAction^ action)
							{
								while (true) {
									auto task = Concurrency::create_task(dataReader->LoadAsync(sizeof(unsigned int))).then(
										[=](unsigned int bytesLoaded)
									{
										uint32_t packSize = dataReader->ReadUInt32();

										auto taskIn = Concurrency::create_task(dataReader->LoadAsync(packSize)).then(
											[=](unsigned int bytesLoaded)
										{
											//read pack code
											Platform::Array<byte>^ packCode = ref new Platform::Array<byte>(4);
											dataReader->ReadBytes(packCode);
											//read num slices
											uint32_t numSlices = dataReader->ReadUInt32();
											//read num channels
											uint32_t numChannels = dataReader->ReadUInt32();
											//read data
											Platform::Collections::Vector<float>^ vec = ref new Platform::Collections::Vector<float>();
											std::vector<float>* stdVec = new std::vector<float>();
											for (int j = 0; j < numChannels; ++j) {
												float value = float(dataReader->ReadInt16());
												vec->Append(value);
												stdVec->push_back(value);
											}
											this->m_main->UpdatElectrodeValues(stdVec);
											Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
												CoreDispatcherPriority::High,
												ref new Windows::UI::Core::DispatchedHandler([this, vec]() {
												
												this->chartManager->AddPointsToAllCharts(vec);
												this->chartManager->Update();
											})
											);

											
											
										});
										taskIn.wait();
									});
									task.wait();
								}
							});

							auto m_SocketLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

							
						});
						//------

					});
				});
				Windows::UI::Popups::MessageDialog^ md = ref new Windows::UI::Popups::MessageDialog(L"Connected");
				md->ShowAsync();

			}
			catch (...)
			{
				Windows::UI::Popups::MessageDialog^ md = ref new Windows::UI::Popups::MessageDialog(L"No connection");
				md->ShowAsync();
			}
		});




		/*Windows::Networking::Sockets::MessageWebSocket^ messageWebSocket = ref new Windows::Networking::Sockets::MessageWebSocket();
		create_task(this->socket->ConnectAsync(hostName, port))
			.then([this](task<void> previousTask) -> task<void>
		{
			try
			{
				previousTask.get();
				return create_task([] {});
				//websocket->Close(1000, "Application caused the connection to close.");
			}
			catch (Exception^ ex)
			{
				::OutputDebugString(L"Couldn't connect to websocket!");
			}

		}).then([this](task<void> previousTask) {
			try {
				previousTask.get();
			}
			catch (const task_canceled&) {
				::OutputDebugString(L"Couldn't connect to websocket!");
			}
			catch (Exception^ exception)
			{
				
				::OutputDebugString(L"Couldn't connect to websocket!");
			}
		});*/
		


	}
	else {
		//TODO. RECREATE
	}
}



// not complited
void DirectXPage::btnCreate3DTransferFunction(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	bool PivotIsAlreadyCreated = false;
	for (int i = 0; i < SwapChainsPivot->Items->Size; ++i) {
		if (((PivotItem^)(SwapChainsPivot->Items->GetAt(i)))->Name == "PivotItem3DTransferFunction") {
			PivotIsAlreadyCreated = true;
		}
	}
	if (!PivotIsAlreadyCreated) {
		PivotItem^ newPivotItem = ref new PivotItem();
		SwapChainPanel^ TransferFunction2DSwapChainPanel = ref new SwapChainPanel();
		TransferFunction2DSwapChainPanel->Name = "TransferFunction3DSwapChainPanel";
		newPivotItem->Content = TransferFunction2DSwapChainPanel;
		newPivotItem->Name = "PivotItem3DTransferFunction";
		newPivotItem->Header = "3D Transfer Function";
		SwapChainsPivot->Items->Append(newPivotItem);

		m_deviceResources->CatchSwapChainPanel(TransferFunction2DSwapChainPanel, "SwapChain3DTransferFunction");
	}
	else {
		//TODO. RECREATE HIST
	}
}


void DirectXPage::SwapChainsPivotSelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e) {
	auto name = ((PivotItem^)SwapChainsPivot->SelectedItem)->Name;
	if (name == "MainPivotItem") {
		m_main->SetCurrentSwapChain(MAIN);
		HistogramNormalizationSlider->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		if (this->m_main->isSurfaceTextureLoaded) 
			SurfaceThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
		if (this->m_main->isBrainTextureLoaded) {
			BrainThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
			EegActivityNormalization->Visibility = Windows::UI::Xaml::Visibility::Visible;
			EegActivityExp->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}
	}
	else if (name == "PivotItem2DTransferFunction") {
		m_main->SetCurrentSwapChain(TRANSFER_FUNCTION_2D);
		HistogramNormalizationSlider->Visibility = Windows::UI::Xaml::Visibility::Visible;
		SurfaceThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		BrainThresholdSlider->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		EegActivityNormalization->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		EegActivityExp->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}
}

void DirectXPage::TF2DHistPointerWheelChanged(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e) {
	//m_main->m_sceneRenderer->AddToTF2DScale(e->GetCurrentPoint(SwapChainsPivot)->Properties->MouseWheelDelta); //change SwapChainsPivot
	if (m_coreInputIsReady2) {
		int mouseWheelData = e->CurrentPoint->Properties->MouseWheelDelta;
		m_main->wheelPointerLoactionX = e->CurrentPoint->Position.X;
		m_main->wheelPointerLoactionY = e->CurrentPoint->Position.Y;
		m_main->wheelAcc += mouseWheelData;
	}
}


void DirectXPage::OnPointerPressedTF2D(Object^ sender, PointerEventArgs^ e)
{
	m_coreInput2->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::SizeAll, 0);
	this->pointerPrX = e->CurrentPoint->Position.X;
	this->pointerPrY = e->CurrentPoint->Position.Y;
	// is tracking
	m_main->isTr = true;
	if (m_main->activeTissueElement != nullptr) {
		if (m_main->activeTissueElement->GetPointsNum() < 3)
			m_main->AddPointToActiveTissue(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
	}
}

void DirectXPage::OnPointerMovedTF2D(Object^ sender, PointerEventArgs^ e)
{
	// Обновление кода отслеживания указателя.
	if (m_main->isTr)
	{
		m_main->TrackingUpdateTF2D(pointerPrX - e->CurrentPoint->Position.X, pointerPrY - e->CurrentPoint->Position.Y);
		this->pointerPrX = e->CurrentPoint->Position.X;
		this->pointerPrY = e->CurrentPoint->Position.Y;
	}
	if (m_main->activeTissueElement != nullptr) {
		m_main->CursorPositionUpdate(e->CurrentPoint->Position.X, e->CurrentPoint->Position.Y);
	}
}

void DirectXPage::OnPointerReleasedTF2D(Object^ sender, PointerEventArgs^ e)
{
	m_coreInput2->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Arrow, 0);
	m_main->TrackingUpdateTF2D(0, 0);
	// Остановка отслеживания движений указателя при освобождении указателя.
	m_main->isTr = false;
}

void DirectXPage::OnPointerEnteredTF2D(Object^ sender, PointerEventArgs^ e)
{
	m_main->isPointerEntered = true;
}

void DirectXPage::OnPointerExitedTF2D(Object^ sender, PointerEventArgs^ e)
{
	m_main->isPointerEntered = false;
}


void DirectXPage::btnAddTissue(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	TissueElement^ newTissueElement = ref new TissueElement();
	m_main->AddTissue(newTissueElement);
	newTissueElement->Margin = Windows::UI::Xaml::Thickness(2);
	//newTissueElement->GetGrid()->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedTissueElement);
	newTissueElement->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedTissueElement);
	newTissueElement->GetOkButton()->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedOkButtonColorPicker);
	TissuesStackPanel->Children->Append(newTissueElement);
}

void DirectXPage::btnAddElectrode(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ElectrodeElement^ newElectrodeElement = ref new ElectrodeElement(ElectrodeStackPanel->Children->Size);
	m_main->AddElectrode(newElectrodeElement);
	newElectrodeElement->Tapped += ref new TappedEventHandler(this, &DirectXPage::TappedElectrodeElement);
	newElectrodeElement->Margin = Windows::UI::Xaml::Thickness(2);
	ElectrodeStackPanel->Children->Append(newElectrodeElement);

	m_main->AddElectrode();
}

void DirectXPage::TappedTissueElement(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e) {
	m_main->SetActiveTissue((TissueElement^)sender);
	m_main->UpdateColors((TissueElement^)sender);
}

void DirectXPage::TappedElectrodeElement(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e) {
	m_main->SetActiveElectrode((ElectrodeElement^)sender);
}

void DirectXPage::TappedOkButtonColorPicker(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{//sender is button not  tissue element -> doesnt work
	//m_main->UpdateColors((TissueElement^)sender);
}

void DirectXPage::SliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	//to exposure
	float exposure = pow((1000-((Slider^)sender)->Value) / ((Slider^)sender)->Maximum, 4);
	this->m_main->SetHistExposure(exposure);
}

void DirectXPage::SurfaceThresholdSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	this->m_main->UpdateSurfaceThresholdValue(((Slider^)sender)->Value);
}

void DirectXPage::BrainThresholdSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	this->m_main->UpdateBrainThresholdValue(((Slider^)sender)->Value);
}

void DirectXPage::EegActivityNormalizationSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	this->m_main->UpdateEegActivityNormalization(((Slider^)sender)->Value);
}

void DirectXPage::EegActivityExpSliderValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	this->m_main->UpdateEegActivityExp(((Slider^)sender)->Value);
}

void DirectXPage::EegAverageValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::DragCompletedEventArgs^ e) {
	this->m_main->UpdatAverValues(((Slider^)sender)->Value);
}


void DirectXPage::keyRotateMRIPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ e) {
	if (((PivotItem^)SwapChainsPivot->SelectedItem)->Name == "MainPivotItem") {
		if (e->VirtualKey == Windows::System::VirtualKey::Q) this->m_main->UpdateRotationX(0.1);
		else if (e->VirtualKey == Windows::System::VirtualKey::A) this->m_main->UpdateRotationX(-0.1);
		else if (e->VirtualKey == Windows::System::VirtualKey::E) this->m_main->UpdateRotationY(0.1);
		else if (e->VirtualKey == Windows::System::VirtualKey::D) this->m_main->UpdateRotationY(-0.1);
		else if (e->VirtualKey == Windows::System::VirtualKey::W) this->m_main->UpdateRotationZ(0.1);
		else if (e->VirtualKey == Windows::System::VirtualKey::S) this->m_main->UpdateRotationZ(-0.1);
	}
}

