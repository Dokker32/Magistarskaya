//
// ChartList.xaml.cpp
// Implementation of the ChartList class
//

#include "pch.h"
#include "ChartList.xaml.h"

using namespace MRIRenderRCT;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

ChartList::ChartList()
{
	InitializeComponent();
}

void ChartList::Update() {
	for (auto it = charts.begin(); it != charts.end(); ++it) {
		(*it)->Update();
	}
	//if (this->isHardUpdated) this->Update();
	//this->isHardUpdated = false;
}

int ChartList::AddChart(Platform::String^ name) {
	auto newChart = ref new m_Chart();
	this->charts.push_back(newChart);
	newChart->SetId(this->idCounter);
	newChart->SetChartName(name);
	newChart->Name = name;
	++idCounter;
	mainStackPanel->Children->Append(newChart);
	return idCounter-1;
}

bool ChartList::AddPointsToAllCharts(Windows::Foundation::Collections::IVector<float>^ points) {
	if (points->Size != this->charts.size()) return false;
	for (auto it = charts.begin(); it != charts.end(); ++it) {
		(*it)->AddPoint(points->GetAt(it - charts.begin()));
	}
	this->Update();
	return true;
}


void ChartList::StackPanelSizeChangedEvent(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e) {
	this->Update();
	this->isHardUpdated = true;
}