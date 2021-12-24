#pragma once
#include "pch.h"

ref class TransferFunctionApplicationsealed {
public:
	void Initialize();


private:
	Windows::ApplicationModel::Core::CoreApplicationView^ coreApplicationView;
	Windows::UI::ViewManagement::ApplicationView^ applicationView;




};