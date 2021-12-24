#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "TissueElement.xaml.h"
#include "ElectrodeElement.xaml.h"
#include "m_Chart.xaml.h"
#include "ChartList.xaml.h"



// Прорисовывает содержимое Direct2D и 3D на экране.
namespace MRIRenderRCT
{
	

	class MRIRenderRCTMain : public DX::IDeviceNotify
	{
	public:
		MRIRenderRCTMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~MRIRenderRCTMain();
		void CreateWindowSizeDependentResources();
		void StartTracking() { m_sceneRenderer->StartTracking(); }
		void TrackingUpdate(float positionX, float positionY) { pointerLocationX = positionX; pointerLocationY = positionY; }
		void StopTracking() { m_sceneRenderer->StopTracking(); }
		bool IsTracking() { return m_sceneRenderer->IsTracking(); }
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }
		Concurrency::critical_section& GetElectrodeCriticalSection() { return electrodeCriticalSection; }
		void SetCurrentSwapChain(CurrentSwapChain currentSwapChain);
		void AddTissue(TissueElement^ newTissue);
		void AddElectrode(ElectrodeElement^ newTissue);
		void SetActiveTissue(TissueElement^ ActiveTissue);
		void AddPointToActiveTissue(float x, float y);

		void SetActiveElectrode(ElectrodeElement^ ActiveElectrode);		
		bool IsElectrodeActive() { return this->isElectrodeActive; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		//void StartTrackingTF2D() { m_sceneRenderer->StartTrackingTF2D(); }
		void TrackingUpdateTF2D(float shiftX, float shiftY) { xPointerShiftTF2D = shiftX; yPointerShiftTF2D = shiftY; }
		void CursorPositionUpdate(float positionX, float positionY) { xPointerPositionTF2D = positionX; yPointerPositionTF2D = positionY; }
		//void StopTrackingTF2D() { m_sceneRenderer->StopTrackingTF2D(); }
		void UpdateColors(TissueElement^ sender);
		void SetHistExposure(float exposure) { this->m_sceneRenderer->UpdateExposure(exposure); }
		void UpdateBrainThresholdValue(float newValue) { this->m_sceneRenderer->UpdateBrainThresholdValue(newValue); }
		void UpdateEegActivityNormalization(float newValue) { this->m_sceneRenderer->SetEegActivityNormalization(newValue); }
		void UpdateSurfaceThresholdValue(float newValue) { this->m_sceneRenderer->UpdateSurfaceThresholdValue(newValue); }
		void UpdateEegActivityExp(float newValue) { this->m_sceneRenderer->SetEegActivityExp(newValue); }
		void AddElectrode() { this->m_sceneRenderer->SetElectrodeToAdd(); }
		void ElectrodeSelected(int id) {this->m_sceneRenderer->ElectrodeSelected(id);}
		void ElectrodeUnselected() { this->m_sceneRenderer->ElectrodeUnselected(); }
		void UpdateElectrodeMousePos(int x, int y) { this->m_sceneRenderer->UpdateElectrodeMousePos(x, y); }
		void StartTrackingElectrode() { this->m_sceneRenderer->StartTrackingElectrode(); }
		void FinishTrackingElectrode(){ this->m_sceneRenderer->FinishTrackingElectrode(); }
		void UpdateElectrode() { this->m_sceneRenderer->SetElectrodeToUpdate(); }
		void UpdatElectrodeValues(std::vector<float>* input);
		void UpdatAverValues(float input) { this->numAv = input; }
	public:
		void ProcessInput();
		void Update();
		bool Render();

		// Кэшированный указатель на ресурсы устройства.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: замените это собственными визуализаторами содержимого.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;
		Concurrency::critical_section electrodeCriticalSection;

		// Таймер цикла прорисовки.
		DX::StepTimer m_timer;

		// Отслеживание текущего положения указателя ввода.
		float pointerLocationX;
		float pointerLocationY;
		float xPointerShiftTF2D; 
		float yPointerShiftTF2D;
		float xPointerPositionTF2D;
		float yPointerPositionTF2D;
		float wheelAcc;
		float wheelPointerLoactionX = -1;
		float wheelPointerLoactionY = -1;
		

		MRIRenderRCT::TissueElement^ activeTissueElement = nullptr;
		MRIRenderRCT::ElectrodeElement^ activeElectrodeElement = nullptr;

		std::vector<MRIRenderRCT::TissueElement^> tissues;
		std::vector<MRIRenderRCT::ElectrodeElement^> electrodes;

		bool isTr = false;
		bool isTissueActive = false;
		bool isElectrodeActive = false;
		bool isPointerEntered = false;
		bool isSurfaceTextureLoaded = false;
		bool isBrainTextureLoaded = false;

	public:
		CurrentSwapChain currentSwapChain = MAIN;

		int electrodesToAdd = 0;
		int addedElectrodes = 0;

		std::vector<float>* pVecElectrodeValues = nullptr;
		std::vector<std::vector<float>*>* pAvVecElectrodeValues = nullptr;
		float numAv = 10;
		void UpdateRotationX(float x);
		void UpdateRotationY(float y);
		void UpdateRotationZ(float z);
	};
}