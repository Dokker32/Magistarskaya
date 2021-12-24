//
// m_Chart.xaml.h
// Declaration of the m_Chart class
//

#pragma once

#include "m_Chart.g.h"

namespace MRIRenderRCT
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class m_Chart sealed
	{
		
	
	public:		
		m_Chart();
		void AddPoint(double Y);
		void Update();		
		void SetId(int id) { this->id = id; }
		int GetId() { return this->id; }
		void SetChartName(Platform::String^ str) { this->chartName->Text = str; }	

	
	private:
		std::vector<double> points;
		int maxVisNumPoints=3;
		double maxAbsValue = 1;
		bool isHardUpdated = false;
		int id = -1;

	};
}


