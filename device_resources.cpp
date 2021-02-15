#include "pch.h"
#include "device_resources.h"

// Suspension Acions -- Trim

void DeviceResources::Trim() {
	// Call this method when the app suspends.
	// It provides a hint to the driver that the app 
	// is entering an idle state and that temporary buffers
	// can be reclaimed for use by other apps.

	//TODO: DXGI convertion problem

	
	winrt::com_ptr<IDXGIDevice3> dxgi_device;
	mdevice_->QueryInterface(__uuidof(IDXGIDevice3), (void**)&dxgi_device);
	//dxgi_device = mdevice_.as<IDXGIDevice3>();

	dxgi_device->Trim();
}
