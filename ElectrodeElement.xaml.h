//
// ElectrodeElement.xaml.h
// Declaration of the ElectrodeElement class
//

#pragma once

#include "ElectrodeElement.g.h"

namespace MRIRenderRCT
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ElectrodeElement sealed
	{
	public:
		ElectrodeElement(uint32_t);
		bool GetIsSelected() { return this->isSelected; }
		void SetIsSelected(bool isSelected) { this->isSelected = isSelected; }
		Windows::UI::Xaml::Controls::Grid^ GetGrid() { return ElectrodeElementGrid; }
		uint32_t GetId() { return id; }
		void SetName(Platform::String^ name);
	private:
		uint32_t id;
		bool isSelected = false;
	};


}
