#pragma once

// WinRT includes
#include <winrt/Windows.UI.Core.h>



// Classs intention
// How should we use it 
class DeviceResources {
public:
	// Make Device Resources upon Construction
	DeviceResources() {}
	// Somewhat initialization related function
	void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window) {}

	// Suspension Acions -- Trim
	// TODO: What does this Trimming means?
	void Trim() {}
	// Swap Chain presention
	void Present() {}

	void SetLogicalSize(winrt::Windows::Foundation::Size logicalSize) {}
	void SetDpi(float dpi) {}
	void ValidateDevice() {}

private:

};
