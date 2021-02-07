#pragma once

// WinRT includes
#include <winrt/Windows.UI.Core.h>



// Classs intention
// How should we use it 
class DeviceResources {
public:
	DeviceResources() {}

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
	// Device creation and initialization inner methods
	void CreateDeviceIndependantResources() {}
	void CreateDeviceREsources() {}
	void CreateWindowSizeDependantResources() {}


	// D3d member vlaues
	winrt::com_ptr<ID3D11Device3> device_;
	winrt::com_ptr<ID3D11DeviceContext3> context_;
	// TODO: why not use swap chain 2?
	winrt::com_ptr<IDXGISwapChain1>  swap_chain_;



};
