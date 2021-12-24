#include "pch.h"
#include "MRIRenderRCTMain.h"
#include "Common\DirectXHelper.h"

using namespace MRIRenderRCT;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Загружает и инициализирует ресурсы приложения во время его загрузки.
MRIRenderRCTMain::MRIRenderRCTMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), pointerLocationX(0.0f), pointerLocationY(0.0f), xPointerShiftTF2D(0.0f),
yPointerShiftTF2D(0.0f)
{
	// Регистрация для получения уведомлений о том, что устройство потеряно или создано заново
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: замените это инициализацией содержимого своего приложения.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: измените настройки таймера, если требуется режим, отличный от режима по умолчанию с переменным шагом по времени.
	// например, для логики обновления с фиксированным временным шагом 60 кадров/с вызовите:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapasedSeconds(1.0 / 60);
	*/

	this->pAvVecElectrodeValues = new std::vector<std::vector<float>*>();
}

MRIRenderRCTMain::~MRIRenderRCTMain()
{
	// Отмена регистрации уведомлений устройства
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Обновляет состояние приложения при изменении размера окна (например, при изменении ориентации устройства)
void MRIRenderRCTMain::CreateWindowSizeDependentResources() 
{
	// TODO: замените это инициализацией содержимого вашего приложения в зависимости от размера.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void MRIRenderRCTMain::StartRenderLoop()
{
	// Если цикл прорисовки анимации уже выполняется, не запускайте новый поток.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Создайте задачу, которая будет выполняться в фоновом потоке.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Вычислите обновленный кадр и отобразите его однократно в каждом вертикальном интервале очистки.
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present(this->currentSwapChain);
			}
		}
	});

	// Выполнение задачи в выделенном высокоприоритетном фоновом потоке.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void MRIRenderRCTMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

// Обновляет состояние приложения один раз за кадр.
void MRIRenderRCTMain::Update() 
{
	ProcessInput();

	// Обновление объектов сцены.
	m_timer.Tick([&]()
	{
		// TODO: замените это функциями обновления содержимого своего приложения.
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

void MRIRenderRCT::MRIRenderRCTMain::UpdateColors(TissueElement^ sender)
{
	m_sceneRenderer->GetTissueFiguresManager()->UpdateColors(sender);
}

// Обработка всех входных данных от пользователя перед обновлением состояния
void MRIRenderRCTMain::ProcessInput()
{
	// TODO: добавьте здесь покадровую обработку входных данных.
	m_sceneRenderer->TrackingUpdate(pointerLocationX, pointerLocationY);
	m_sceneRenderer->UpdateTF2DEdgesView(xPointerShiftTF2D, yPointerShiftTF2D);
	this->xPointerShiftTF2D = 0;
	this->yPointerShiftTF2D = 0;
	this->m_sceneRenderer->AddToTF2DScale(wheelAcc, wheelPointerLoactionX, wheelPointerLoactionY);
	this->wheelAcc = 0;
	this->wheelPointerLoactionX = -1; //null
	this->wheelPointerLoactionY = -1;
	if (this->isTissueActive && isPointerEntered) {
		m_sceneRenderer->SetPointToRender(this->xPointerPositionTF2D, this->yPointerPositionTF2D, activeTissueElement->GetTissueColor());
	}
	//delete 2 rows
	//this->pVecElectrodeValues = new std::vector<float>();
	//this->pVecElectrodeValues->push_back(rand());
	/*if (this->pVecElectrodeValues != nullptr) {
		m_sceneRenderer->ChangeElectrodeValues(*this->pVecElectrodeValues);
		//delete this->pVecElectrodeValues;
	}*/
	//
	if (this->pVecElectrodeValues != nullptr) {
		int av = numAv;
		if (pAvVecElectrodeValues->size() < av) av = pAvVecElectrodeValues->size();
		std::vector<float> vecc;
		for (int i = 0; i < pVecElectrodeValues->size(); ++i) {
			vecc.push_back(0);
		}
		for (int i = 0; i < av; ++i) {
			for (int j = 0; j < pAvVecElectrodeValues->at(pAvVecElectrodeValues->size() - 1 - i)->size(); ++j) {
				vecc.at(j) += pAvVecElectrodeValues->at(pAvVecElectrodeValues->size() - 1 - i)->at(j)/av;
			}
		}
		m_sceneRenderer->ChangeElectrodeValues(vecc);
	}
	
}

// Прорисовывает текущий кадр в соответствии с текущим состоянием приложения.
// Возвращает значение true, если кадр прорисован и готов к отображению.
bool MRIRenderRCTMain::Render() 
{
	// Не пытаться выполнять какую-либо прорисовку до первого обновления.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();
	switch (this->currentSwapChain)
	{
	case MAIN:
		{// Выполните сброс окна просмотра для нацеливания на весь экран.
			auto viewport = m_deviceResources->GetScreenViewport();
			context->RSSetViewports(1, &viewport);

			// Сброс целевых объектов прорисовки в экран.
			ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
			context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

			// Очистка заднего буфера и представления трафарета глубины.
			context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
			context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			// Прорисовка объектов сцены.
			// TODO: замените это функциями прорисовки содержимого своего приложения.
			bool isRendered = m_sceneRenderer->Render();
			//m_fpsTextRenderer->Render();

			return isRendered;
		}
		break;
	case TRANSFER_FUNCTION_2D:
	{
		auto swapChainWarp = m_deviceResources->getSwapChainWarpHandler()->getWarp("SwapChain2DTransferFunction");
		
		auto viewport = m_deviceResources->GetScreenViewport();
		context->RSSetViewports(1, &viewport);

		// Сброс целевых объектов прорисовки в экран.
		ID3D11RenderTargetView* const targets[1] = { swapChainWarp->renderTargetView.Get() };
		context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());		
		
		// Очистка заднего буфера и представления трафарета глубины.
		context->ClearRenderTargetView(swapChainWarp->renderTargetView.Get(), DirectX::Colors::Black);
		context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Прорисовка объектов сцены.
		// TODO: замените это функциями прорисовки содержимого своего приложения.
		bool isRendered = m_sceneRenderer->Render();
		m_fpsTextRenderer->Render();

		return isRendered;
	}
		break;
	default:
		break;
	}
	
}

// Уведомляет визуализаторы о том, что ресурсы устройства необходимо освободить.
void MRIRenderRCTMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Уведомляет визуализаторы о том, что ресурсы устройства можно создать заново.
void MRIRenderRCTMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void MRIRenderRCTMain::SetCurrentSwapChain(CurrentSwapChain currentSwapChain) {
	this->currentSwapChain = currentSwapChain;
	this->m_sceneRenderer->SetCurrentSwapChain(currentSwapChain);
}


void MRIRenderRCTMain::AddTissue(TissueElement^ newTissue) {
	if (!this->tissues.empty()) {
		for (int i = 0; i < INT_MAX-1; ++i) {
			bool isIdOccupied = false;
			for (auto iter = this->tissues.begin(); iter != this->tissues.end(); ++iter) {
				if (i == (*iter)->GetId()) {
					isIdOccupied = true;
				}
			}
			if (isIdOccupied == false) {
				newTissue->SetId(i);
				newTissue->SetName(ref new Platform::String( (L"New Tissue " + to_wstring(i)).c_str() ));
				i = INT_MAX-1;
			}
		}
	}
	else {
		newTissue->SetId(0);
		newTissue->SetName(ref new Platform::String(L"New Tissue"));
	}

	this->tissues.push_back(newTissue);
	this->m_sceneRenderer->GetTissueFiguresManager()->AddTissueFigure(newTissue->GetId(), newTissue->GetColor());

}

void MRIRenderRCTMain::AddElectrode(ElectrodeElement^ newElectrode) {
	//Concurrency::critical_section::scoped_lock lock(electrodeCriticalSection);
	this->electrodes.push_back(newElectrode);
	while (addedElectrodes<electrodesToAdd) {
		this->m_sceneRenderer->numElectrodesToAdd += 1;
		++addedElectrodes;
	}

}


void MRIRenderRCTMain::SetActiveTissue(TissueElement^ ActiveTissue) {
	if (!ActiveTissue->GetIsSelected()) {
		for (auto it = tissues.begin(); it != tissues.end(); ++it) {
			(*it)->SetIsSelected(false);
			(*it)->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
		}
		ActiveTissue->SetIsSelected(true);
		ActiveTissue->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::LightGray);
		this->isTissueActive = true;
		this->activeTissueElement = ActiveTissue;
	}
	else {
		ActiveTissue->SetIsSelected(false);
		ActiveTissue->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
		this->isTissueActive = false;
		this->activeTissueElement = nullptr;
	}

}


void MRIRenderRCTMain::SetActiveElectrode(ElectrodeElement^ ActiveElectrode) {
	if (!ActiveElectrode->GetIsSelected()) {
		for (auto it = electrodes.begin(); it != electrodes.end(); ++it) {
			(*it)->SetIsSelected(false);
			(*it)->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
		}
		ActiveElectrode->SetIsSelected(true);
		ActiveElectrode->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::LightGray);
		this->isElectrodeActive = true;
		this->activeElectrodeElement = ActiveElectrode;
		this->m_sceneRenderer->ElectrodeSelected(ActiveElectrode->GetId()+1);
	}
	else {
		ActiveElectrode->SetIsSelected(false);
		ActiveElectrode->GetGrid()->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::White);
		this->isElectrodeActive = false;
		this->activeElectrodeElement = nullptr;
		this->m_sceneRenderer->ElectrodeUnselected();
	}
}


void MRIRenderRCTMain::AddPointToActiveTissue(float x, float y) {
	this->activeTissueElement->AddPoint(Windows::Foundation::Point(x,y));
	this->m_sceneRenderer->UpdateInstancePointBuffer(&tissues);
	float xEdge = m_sceneRenderer->TF2DScaleStruct.xEdge;
	float yEdge = m_sceneRenderer->TF2DScaleStruct.yEdge;
	float scale = m_sceneRenderer->TF2DScaleStruct.scale;
	x /= m_deviceResources->GetLogicalSize().Width;
	y /= m_deviceResources->GetLogicalSize().Height;	
	/*Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
		ref new Windows::UI::Core::DispatchedHandler([xEdge]() {
		Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(to_wstring(xEdge).c_str()));
		msg->ShowAsync(); }));*/
	x /= scale;
	x += xEdge;
	y /= scale;
	y += yEdge;
	x *= m_deviceResources->GetLogicalSize().Width;;
	y *= m_deviceResources->GetLogicalSize().Height;
	this->m_sceneRenderer->GetTissueFiguresManager()->AddPointToTissue(x,y, this->activeTissueElement->GetId());

	/*// -> to tex coord coordinates 
	pos.xy = ((pos.xy + screenSpaceVertexPosition) + 1.0f) / 2.0f;
	// -> shift edges
	pos.xy += -float2(xEdge, 1.0f - 1.0f / scale - yEdge);
	// -> transsform
	pos.xy *= scale;
	// -> to scren space coordinates
	pos.xy = (pos.xy - 0.5f) * 2.0f;*/

	/*Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Low,
			ref new Windows::UI::Core::DispatchedHandler([this]() {
			Windows::UI::Popups::MessageDialog^ msg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(to_wstring(GetPointsNum()).c_str()));
			msg->ShowAsync();
*/
	
}


void MRIRenderRCTMain::UpdateRotationX(float x) {
	this->m_sceneRenderer->AddToRotationX(x);
}
void MRIRenderRCTMain::UpdateRotationY(float y) {
	this->m_sceneRenderer->AddToRotationY(y);
}
void MRIRenderRCTMain::UpdateRotationZ(float z) {
	this->m_sceneRenderer->AddToRotationZ(z);
}

void MRIRenderRCTMain::UpdatElectrodeValues(std::vector<float>* input) {
	//if (this->pVecElectrodeValues != nullptr) delete this->pVecElectrodeValues;
	this->pVecElectrodeValues = input; 
	this->pAvVecElectrodeValues->push_back(input);
}