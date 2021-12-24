#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "dicom3D.h"
#include "InterfaceElement.h"
#include "helper.h"
#include "TissueElement.xaml.h"


enum RenderState {PAUSE_STATE, READY_STATE, SUSPENDED_STATE, NEW_TEXTURE_LOADING_RENDER_STATE};

struct ElectrodeStruct {
	int id;
	int texPos;
	float value;
};

struct Rotator {
	float x = 0;
	float y = 0;
	float z = 0;
};

struct ScaleStruct {
	float scale;
	float yEdge;
	float xEdge;
	float exposure;

	ScaleStruct(float scale, float yEdge, float xEdge, float tonalCompression) 
		: scale(scale), yEdge(yEdge), xEdge(xEdge), exposure(tonalCompression) {}
};

struct VSPointStruct {
	float xPosition;
	float yPosition;
	DirectX::XMFLOAT2 padding;
};

struct PSPointStruct {
	float red;
	float green;
	float blue;
	float trash;
	float xCenter;
	float yCenter;
	float trash1;
	float trash2;
};

struct PointInstStruct {
	DirectX::XMFLOAT2 pos;
	DirectX::XMFLOAT3 color;
};

struct instStruct {
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT2 center;
};

struct linesDataStruct {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
};

struct TissueFigureLine {
	TissueFigureLine(Windows::Foundation::Point startPos, Windows::Foundation::Point endPos, unsigned int id) :
		startPos(startPos), endPos(endPos), lineId(id) {};

	Windows::Foundation::Point startPos;
	Windows::Foundation::Point endPos;
	unsigned int lineId;
};

struct TissueFigurePoint {
public:
	TissueFigurePoint(float x,float y, unsigned int id) {
		position.X = x;
		position.Y = y;
		pointId = id;	
	};
	Windows::Foundation::Point GetPosition() { return this->position; }
private:
	Windows::Foundation::Point position;
public:
	unsigned int pointId;
};

class TissueFigure2D {
public:
	unsigned int tissueId;
	std::vector<TissueFigurePoint> points;
	std::vector<TissueFigureLine> lines;
	Windows::UI::Color color;
	std::shared_ptr<DX::DeviceResources> deviceResources;
	float opacity;

	//DirectX resources
	Microsoft::WRL::ComPtr<ID3D11Buffer>		pointsInstanceBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		linesVertexBuffer;
public:
	TissueFigure2D(unsigned int tissueId, Windows::UI::Color color, std::shared_ptr<DX::DeviceResources> deviceResources) : 
		tissueId(tissueId), deviceResources(deviceResources), color(color){}
	void AddPoint(float x, float y, std::shared_ptr<DX::DeviceResources> deviceResources);
	void UpdateBuffers();
	void EditPointPosition();
	unsigned int GetPointsNum() { return this->points.size(); }
	unsigned int GetLinesNum() { return this->lines.size(); }
	unsigned int GetTissueId() { return this->tissueId; }
	void Render(DX::DeviceResources* deviceResources);
	void CheckCursorCollision(int x, int y);
	void UpdateColor(Windows::UI::Color color) { color = color; }
	ID3D11Buffer* GetPointsInstanceBuffer() { return this->pointsInstanceBuffer.Get(); }
	ID3D11Buffer* GetLinesVertexBuffer() { return this->linesVertexBuffer.Get(); }
	void SetColor(Windows::UI::Color color) { this->color = color; this->UpdateBuffers(); }
	void SetOpacity(float opacity) { this->opacity = opacity; }
	
};

//keeps set of tissue figures. init resources. draw 
class TissueFiguresManager {
public:
	TissueFiguresManager(std::shared_ptr<DX::DeviceResources> deviceResources);
	void Render(Microsoft::WRL::ComPtr <ID3D11Buffer> cbTF2DScale, Microsoft::WRL::ComPtr <ID3D11Buffer> cbPSPointStuct);
	void AddTissueFigure(unsigned int tissueId, Windows::UI::Color color) {
		this->tissueFigures.push_back(TissueFigure2D(tissueId, color, deviceResources));
	}
	void AddPointToTissue(float x, float y, unsigned int tissueId);
	void UpdateColors(MRIRenderRCT::TissueElement^ sender);


	ID3D11Texture2D* TransferFunction2D;
	ID3D11ShaderResourceView* srvTransferFunction2D;
	bool UpdateFigCom = false;
public:
	std::vector<TissueFigure2D> tissueFigures;

private:
	std::shared_ptr<DX::DeviceResources>		deviceResources;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexPlaneBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	pointLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	linesLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	VSPointInst;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	PSPointInst;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	VSLines;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	PSLines;
};

void FillFigure(ID3D11Texture2D* outputTexture, TissueFigure2D* figure, std::shared_ptr<DX::DeviceResources> deviceResources);

class Sensor {
public: 
	Sensor(uint32_t id) { this->id = id; };
	void SetPosition(DirectX::XMFLOAT3 newPos);
	DirectX::XMFLOAT3 GetPosition();
private:
	//sensor position in the local mri space
	uint32_t id;
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(-1,-1,-1);
	bool isInitiated = false;
	bool isSelected = false;
};

class SensorManager {
public:
	SensorManager(std::shared_ptr<DX::DeviceResources> deviceResources);
	void AddSensor(uint32_t id, Windows::UI::Color color) {
		this->sensors.push_back(Sensor(id));
	}
	void chooseSensor(int id);

private:
	std::vector<Sensor> sensors;
	uint32_t activeSensorId;
};

namespace MRIRenderRCT
{
	// Этот пример визуализатора создает экземпляр базового конвейера прорисовки.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void StartTracking();
		void TrackingUpdate(float positionX, float positionY);
		void StopTracking();
		bool IsTracking() { return m_tracking; }
		ID3D11Texture3D** getMRITexturePointer();
		void DoRender();
		void UpdateMRITextureSRV();
		void setRealTimeRender() { isRealTimeRender = true; }
		void unsetRealTimeRender() { isRealTimeRender = false; }
		void MRITextureSetSizeParameters(unsigned int height, unsigned int width, unsigned int length);
		void RenderRCPositions();
		void RenderHist();
		void RenderTF2DAreas();
		void SetCurrentSwapChain(CurrentSwapChain currentSwapChain);
		void ShowWaitingState();
		void Create2DTransferFunctionResources();
		void AddToTF2DScale(float wheel, float wheelPointerLocationX, float wheelPointerLocationY);
		void UpdateTF2DEdgesView(float xEdge, float yEdge);
		//if tissue active, render point in the cycle
		void SetPointToRender(float x, float y, Windows::UI::Color color);
		void UpdateInstancePointBuffer(std::vector<TissueElement^>* input);
		//Concurrency::critical_section& GetElectrodeCriticalSection() { return electrodeCriticalSection; }


		void RenderPointUnderCursor();
		void RenderPointsTF2D();
		void UpdateExposure(float exposure);
		void UpdateBrainThresholdValue(float newValue);
		void UpdateSurfaceThresholdValue(float newValue);
		void SetEegActivityNormalization(float value);
		void SetEegActivityExp(float value);

		void UpdateMRITextureFromRawDataFloat(float* inputRawData, int xSize, int ySize, int zSize);
		void UpdateSecondMRITextureFromRawDataFloat(float* inputRawData, int xSize, int ySize, int zSize);

		TissueFiguresManager* GetTissueFiguresManager() { return &(this->tissueFiguresManager); }

		void SetElectrodeToAdd();
		void SetElectrodeToUpdate() { isUpdateElectrode = true; }
		void UpdateElectrodeMousePos(int mousePosX, int mousePosY) { 
			elMousePosX = mousePosX / this->m_deviceResources->GetLogicalSize().Width * this->m_deviceResources->getRenderTargetSize().Width;
			elMousePosY = mousePosY / this->m_deviceResources->GetLogicalSize().Height * this->m_deviceResources->getRenderTargetSize().Height;
		}
		void ElectrodeSelected(int id) { this->idOfActiveElectrode = id; MRIParamStructInst.selectedElectrode = id; this->isUpdateMRIParamBuffer = true;}
		void ElectrodeUnselected() { this->idOfActiveElectrode = -1; MRIParamStructInst.selectedElectrode = -1; this->isUpdateMRIParamBuffer = true;}
		void UpdateElectrodeValues();
		void ChangeElectrodeValues(std::vector<float> input);
		void AddElectrode();
		void TrackingElectrode();
		void UpdateElectrode();
		void StartTrackingElectrode() { this->isTrackingElectrode = true; }
		void FinishTrackingElectrode() { this->isTrackingElectrode = false; }

		

		bool MRITextureReady;

		//Concurrency::critical_section electrodeCriticalSection;

	private:
		void Rotate();

		

	private:
		// Кэшированный указатель на ресурсы устройства.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Ресурсы Direct3D 
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayoutPos;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBufferPos;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferPos;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShaderPos;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShaderPos;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	PS2DTF;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		constantDicomParametersBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brainTexture3DView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mainTexture3DView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> testTexture3DView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> testSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mainSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> notInterSampler;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mBack;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mFront;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtvBack;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtvFront;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBack;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvFront;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	VSPoint;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	PSPoint;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	VSPointInst;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	PSPointInst;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	PSAreas;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> renderResult;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvRenderResult;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uavRenderResult;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> CSMRITF;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> CSMRIWithAddTexture;
		Microsoft::WRL::ComPtr<ID3D11Buffer> cbMRIParam;
		Microsoft::WRL::ComPtr<ID3D11Buffer> cbTF2DScale;
		Microsoft::WRL::ComPtr<ID3D11Buffer> cbVSPointStuct;
		Microsoft::WRL::ComPtr<ID3D11Buffer> cbPSPointStruct;
		//
		Microsoft::WRL::ComPtr<ID3D11Texture3D> TEST;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TESTView;
		//
		Microsoft::WRL::ComPtr<ID3D11Texture2D> TransferFunction2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvTransferFunction2D;

		std::shared_ptr<InterfaceElement> histo;
		// Системные ресурсы для геометрии куба.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		dicomParametersForShaders			dicomParametersBufferData;
		uint32	m_indexCount;
		uint32	m_indexCountPos;
		uint32 squadIndexCount;
		//instance point
		int instanceCount = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		instanceBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	instanceInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		instanceVertexBuffer;
		//my
		Dicom3D dicomFile;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> MRITexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMRITexture;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> MRITextureBlurred;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMRITextureBlurred;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> secondMRITexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSecondMRITexture;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> secondMRITextureBlurred;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSecondMRITextureBlurred;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> MRIGradTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMRIGradTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> MRIScalarNodeTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMRIScalarNodeTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Hist2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvHist2D;
		unsigned int MRITextureWidth;
		unsigned int MRITextureHeight;
		unsigned int MRITextureLength;

		float MRIMaxScalarValue;
		float MRIMaxGradMagn;

		MRIRenderRCT::MRIParamStruct MRIParamStructInst;

		void InitializationRenderStates();
		
		public: ScaleStruct TF2DScaleStruct;
		private: VSPointStruct VSPointStructInst;
		PSPointStruct PSPointStructInst;

		unsigned int outputTextureSizeX;
		unsigned int outputTextureSizeY;
		// Переменные, используемые с циклом прорисовки.
		bool	m_loadingComplete = false;
		bool	TF2DInitialized = false;
		bool	m_DoRender;
		float	m_degreesPerSecond;
		bool	m_tracking;
		bool	isRealTimeRender;
		bool	isRenderPointUnderCursor;
		bool isTF2DCreated = false;
		bool isTFReady = false;

		//
		TissueFiguresManager tissueFiguresManager;
		//
		RenderState renderState = PAUSE_STATE;
		CurrentSwapChain currentSwapChain = MAIN;

		//
		float TF2DShiftX;
		float TF2DShiftY;

		//
		bool isUpdateMRIParamBuffer = false;

		//create texture from raw data		
		float* rawTextureData;
		int xSizeTextureRaw;
		int ySizeTextureRaw;
		int zSizeTextureRaw;
		int xSizeSecondTextureRaw;
		int ySizeSecondTextureRaw;
		int zSizeSecondTextureRaw;
		bool createFromRaw = false;
		bool createSecond = false;
		bool useSecondTexture = false;

		//electrodes
		uint32_t numElectrodes = 0;
		float* electrodeRawTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture1D> electrodeTexture;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uavElectrodeTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture1D> electrodeValTexture;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uavElectrodeValTexture;
		std::vector<ElectrodeStruct> electrodeList;
		int idOfActiveElectrode = -1;
		int elMousePosX = 0;
		int elMousePosY = 0;
		bool isAddElectrode = false;

		std::vector<float> electrodeValues;

		bool isTrackingElectrode = false;
		bool isUpdateElectrode = false;

	public:
		int numElectrodesToAdd = 0;
		int addedElectrodes = 0;
		//rotation
		Rotator rotator;
		void AddToRotationX(float x);
		void AddToRotationY(float y);
		void AddToRotationZ(float z);


		//delete
		//std::chrono::time_point<std::chrono::high_resolution_clock>* start = nullptr;

	};
}



