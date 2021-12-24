#include "pch.h"
#include "TransferFunction.h"


using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::ViewManagement;

void TransferFunctionApplicationsealed::Initialize() {

	coreApplicationView = CoreApplication::CreateNewView();
	applicationView = ApplicationView::GetForCurrentView();
	

}