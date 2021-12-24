#include "pch.h"
#include "ChartManager.h"


void ChartManager::Update() {
	Concurrency::critical_section::scoped_lock lock(m_criticalSection);

	this->chartList->Update();
}


void ChartManager::AddPointsToAllCharts(Windows::Foundation::Collections::IVector<float>^ vec) {
	Concurrency::critical_section::scoped_lock lock(m_criticalSection);

	this->chartList->AddPointsToAllCharts(vec);
}
