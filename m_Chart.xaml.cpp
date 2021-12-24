//
// m_Chart.xaml.cpp
// Implementation of the m_Chart class
//

#include "pch.h"
#include "m_Chart.xaml.h"

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

m_Chart::m_Chart()
{
	InitializeComponent();
}

void m_Chart::Update() {
	
	this->canvas->Children->Clear();
	int visNumPoints = this->maxVisNumPoints;
	if (points.size() < this->maxVisNumPoints) visNumPoints = points.size();
	double maxX = this->canvas->ActualWidth;
	double maxY = this->canvas->ActualHeight / 2;
	double currX = 0;
	double prevY = maxY - *(points.end() - visNumPoints) / this->maxAbsValue * maxY;
	for (auto it = points.end() - visNumPoints + 1; it != points.end(); ++it) {
		Windows::UI::Xaml::Shapes::Line^ line = ref new Windows::UI::Xaml::Shapes::Line();
		line->Stroke = ref new SolidColorBrush(Windows::UI::Colors::Red);
		line->Y1 = prevY;
		line->X1 = currX;
		currX += maxX / (this->maxVisNumPoints - 1);
		line->X2 = currX;
		line->Y2 = maxY - *(it) / this->maxAbsValue * maxY;
		prevY = line->Y2;
		this->canvas->Children->Append(line);
		
	}
}

void m_Chart::AddPoint(double Y) {
	points.push_back(Y);
	if (abs(Y) > this->maxAbsValue) maxAbsValue = abs(Y);
}


