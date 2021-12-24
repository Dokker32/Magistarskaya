//
// ChartList.xaml.h
// Declaration of the ChartList class
//

#pragma once

#include "ChartList.g.h"
#include "m_Chart.xaml.h"

namespace MRIRenderRCT
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ChartList sealed
	{
	public:
		ChartList();
		int AddChart(Platform::String^ name);
		bool AddPointsToAllCharts(Windows::Foundation::Collections::IVector<float>^ points);
		void Update();		
	private:
		void StackPanelSizeChangedEvent(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		std::vector<m_Chart^> charts;
		int idCounter=0;
		bool isHardUpdated = false;
	};
}
