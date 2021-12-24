//
// TissueElement.xaml.h
// Declaration of the TissueElement class
//

#pragma once

#include "TissueElement.g.h"
#include "pch.h"
#include "helper.h"

namespace MRIRenderRCT
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class TissueElement sealed
	{
	public:
		TissueElement();
		void SetName(Platform::String^ name) { tbTissueName->Text = name; }
		//Platform::String GetName() {return  tbTissueName->Text;	}

		void SetId(int id) { this->id = id; }
		int	GetId() { return this->id; }
		bool GetIsSelected() {return this->isSelected;}
		void SetIsSelected(bool isSelected) { this->isSelected = isSelected; }
		Windows::UI::Xaml::Controls::Grid^ GetGrid() {return TissueElementGrid;}
		Windows::UI::Color GetTissueColor() { return this->color;}
		void AddPoint(Windows::Foundation::Point point) { this->points->Append(point);
		}
		Windows::Foundation::Collections::IVector<Windows::Foundation::Point>^ GetPointsP() {return this->points; }
		int GetPointsNum() { return this->points->Size;}
		Windows::UI::Color GetColor() { return this->color; }
		Windows::UI::Xaml::Controls::Button^ GetOkButton() { return OkButton; }
		float GetOpacity() { return this->opacity; }
	private:
		void TextBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
		void AlphaValueChangedEvent(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
		void EnterPressed(Platform::Object^ sender, Windows::UI::Core::KeyEventArgs^ e);
		void RectangleTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void BtnColorClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ConfirmColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CancelColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	
	private:
		Windows::UI::Color color;
		float opacity = 0.05f;
		int id = -1; //invalid id
		bool isSelected = false;
		Platform::Collections::Vector<Windows::Foundation::Point>^ points;


	};
}
