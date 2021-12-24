//
// TissueElement.xaml.cpp
// Implementation of the TissueElement class
//

#include "pch.h"
#include "TissueElement.xaml.h"

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

TissueElement::TissueElement()
{
	InitializeComponent();
	this->color = ((SolidColorBrush^)clrBtn->Background)->Color;
	this->points = ref new Platform::Collections::Vector<Windows::Foundation::Point>();
}


void MRIRenderRCT::TissueElement::TextBox_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{

}

void MRIRenderRCT::TissueElement::AlphaValueChangedEvent(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e) {
	auto stringSize = std::size_t(((TextBox^)sender)->Text->Length());
	auto value = std::stof(((TextBox^)sender)->Text->Data(), &stringSize);
	if (value > 1) value = 1.0;
	if (value < 0) value = 0.0;
	((TextBox^)sender)->Text = value.ToString();
	this->opacity = value;
}

void MRIRenderRCT::TissueElement::EnterPressed(Platform::Object^ sender, Windows::UI::Core::KeyEventArgs^ e)
{	
	if (e->VirtualKey == Windows::System::VirtualKey::Enter)
	{
		auto stringSize = std::size_t(((TextBox^)sender)->Text->Length());
		auto value = std::stof(((TextBox^)sender)->Text->Data(), &stringSize);
		if (value > 1) value = 1.0;
		if (value < 0) value = 0.0;
		((TextBox^)sender)->Text = value.ToString();
		this->opacity = value;
	}
}



void TissueElement::RectangleTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e) {
		FlyoutBase::ShowAttachedFlyout((FrameworkElement^)sender);
}

void TissueElement::ConfirmColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	clrBtn->Background = ref new Windows::UI::Xaml::Media::SolidColorBrush(myColorPicker->Color);
	MRIRenderRCT::TissueElement::mFlyout->Hide();
	this->color = myColorPicker->Color;
}

void TissueElement::CancelColor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	MRIRenderRCT::TissueElement::mFlyout->Hide();
}

void TissueElement::BtnColorClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	((Button^)sender)->Flyout->ShowAt((FrameworkElement^)sender);
}


