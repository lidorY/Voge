#pragma once

// WinRT includes
#include <winrt/Windows.UI.Core.h>



// Classs intention
// How should we use it

interface IDeviceNotify
{
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceRestored() = 0;
};

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
	void HandleDeviceLost() {}
	void RegisterDeviceNotify(IDeviceNotify* deviceNotify) {}

	// Device Accessors.
	winrt::Windows::Foundation::Size GetOutputSize() const { return output_size_; }
	winrt::Windows::Foundation::Size GetLogicalSize() const { return logical_size_; }
	winrt::Windows::Foundation::Size GetRenderTargetSize() const { return render_target_size_; }
	float GetDpi() const { return dpi_; }

	// D3D Accessors.
	winrt::com_ptr<ID3D11Device3> const& GetD3DDevice() const { return device_; }
	ID3D11DeviceContext3* GetD3DDeviceContext() const { return context_.get(); }
	IDXGISwapChain1* GetSwapChain() const { return swap_chain_.get(); }
	D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return feature_level_; }
	ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return render_target_view_.get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depth_stencil_view_.get(); }
	D3D11_VIEWPORT          GetScreenViewport() const { return screen_viewport_; }

private:
	// Device creation and initialization inner methods
	void CreateDeviceIndependantResources() {}
	void CreateDeviceREsources() {}
	void CreateWindowSizeDependantResources() {}


	// Direct3D objects.
	winrt::com_ptr<ID3D11Device3>         device_;
	winrt::com_ptr<ID3D11DeviceContext3>  context_;
	winrt::com_ptr<IDXGISwapChain1>       swap_chain_;

	// Direct3D rendering objects. Required for 3D.
	winrt::com_ptr<ID3D11RenderTargetView>  render_target_view_;
	winrt::com_ptr<ID3D11DepthStencilView>  depth_stencil_view_;
	D3D11_VIEWPORT                          screen_viewport_;

	// Cached reference to the Window.
	winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> window_{ nullptr };


	// Cached device properties.
	D3D_FEATURE_LEVEL                                      feature_level_;
	winrt::Windows::Foundation::Size                       render_target_size_;
	winrt::Windows::Foundation::Size                       output_size_;
	winrt::Windows::Foundation::Size                       logical_size_;
	float dpi_;

	// The IDeviceNotify can be held directly as it owns the DeviceResources.
	IDeviceNotify* m_deviceNotify; 
};
