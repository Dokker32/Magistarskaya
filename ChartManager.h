#pragma once
#include "MRIRenderRCTMain.h"
#include "ChartList.xaml.h"



class ChartManager {
public:
	ChartManager(MRIRenderRCT::ChartList^ chartList) { this->chartList = chartList; }
	~ChartManager(){}

	void Update();
	void AddPointsToAllCharts(Windows::Foundation::Collections::IVector<float>^ vec);

	Concurrency::critical_section m_criticalSection;

private:

	MRIRenderRCT::ChartList^ chartList;
};