#pragma once
#include "pch.h"
#include "dxgi1_3.h"
// WinRT includes
#include <winrt/Windows.UI.Core.h>


using namespace DirectX;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;

	interface IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	class DeviceResources {
	public:
		DeviceResources() noexcept :
			screen_viewport_(),
			feature_level_(D3D_FEATURE_LEVEL_11_0),
			render_target_size_(),
			output_size_(),
			logical_size_(),
			dpi_(-1.0f),
			device_notify_(nullptr)
		{
			CreateDeviceIndependentResources();
			CreateDeviceResources();
		}

		void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window) {
			using namespace winrt::Windows::Graphics::Display;
			DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();
			window_ = window;
			SetDpi(currentDisplayInformation.LogicalDpi());
		}

		// Suspension Acions -- Trim
		void Trim();

		// Swap Chain presention
		void Present() {
			// The first argument instructs DXGI to block until VSync, putting the application
			// to sleep until the next VSync. This ensures we don't waste any cycles rendering
			// frames that will never be displayed to the screen.
			HRESULT hr = swap_chain_->Present(1, 0);

			// Discard the contents of the render target.
			// This is a valid operation only when the existing contents will be entirely
			// overwritten. If dirty or scroll rects are used, this call should be removed.
			context_->DiscardView(render_target_view_.get());
			// Discard the contents of the depth stencil.
			context_->DiscardView(depth_stencil_view_.get());

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) { HandleDeviceLost(); }
			else { winrt::check_hresult(hr); }
		}

		void SetLogicalSize(winrt::Windows::Foundation::Size logicalSize) {
			if (logical_size_ == logicalSize) { return; }

			logical_size_ = logicalSize;
			CreateWindowSizeDependentResources();
		}

		void SetDpi(float dpi) {
			if (dpi_ == dpi) { return; }

			dpi_ = dpi;
			SetLogicalSize(winrt::Windows::Foundation::Size(
				window_.get().Bounds().Width,
				window_.get().Bounds().Height
			));
		}

		void ValidateDevice() {
			// The D3D Device is no longer valid if the default adapter changed since the device
			// was created or if the device has been removed.

			// First, get the information for the default adapter from when the device was created.

			//TODO: DXGI convertion problem

			//winrt::com_ptr<IDXGIDevice3> dxgi_device;
			//dxgi_device = mdevice_.as<IDXGIDevice3>();
			//winrt::com_ptr<IDXGIAdapter> device_adapter;
			//winrt::check_hresult(dxgi_device->GetAdapter(device_adapter.put()));
			//winrt::com_ptr<IDXGIFactory2> device_factory;
			//winrt::check_hresult(device_adapter->GetParent(IID_PPV_ARGS(&device_factory)));
			//winrt::com_ptr<IDXGIAdapter1> previous_default_adapter;
			//winrt::check_hresult(device_factory->EnumAdapters1(0, previous_default_adapter.put()));


			//DXGI_ADAPTER_DESC previous_desc;
			//winrt::check_hresult(previous_default_adapter->GetDesc(&previous_desc));

			//// Next, get the information for the current default adapter.
			//winrt::com_ptr<IDXGIFactory2> current_factory;
			//winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(&current_factory)));
			//winrt::com_ptr<IDXGIAdapter1> current_default_adapter;
			//winrt::check_hresult(current_factory->EnumAdapters1(0, current_default_adapter.put()));


			//DXGI_ADAPTER_DESC current_desc;
			//winrt::check_hresult(current_default_adapter->GetDesc(&current_desc));


			//if (previous_desc.AdapterLuid.LowPart != current_desc.AdapterLuid.LowPart ||
			//	previous_desc.AdapterLuid.HighPart != current_desc.AdapterLuid.HighPart ||
			//	FAILED(mdevice_->GetDeviceRemovedReason()))
			//{
			//	// TODO: this implies we need to release the refernces
			//	// before we call HandleDeviceLost. Is this true or just error in code?

			//	// Release references to resources related to the old device.
			//	dxgi_device = nullptr;
			//	device_adapter = nullptr;
			//	device_factory = nullptr;
			//	previous_default_adapter = nullptr;

			//	// Create a new device and swap chain.
			//	HandleDeviceLost();
			//}
		}

		void HandleDeviceLost() {
			swap_chain_ = nullptr;

			if (device_notify_ != nullptr) { device_notify_->OnDeviceLost(); }

			// Make sure the rendering state has been released.
			context_->OMSetRenderTargets(0, nullptr, nullptr);
			depth_stencil_view_ = nullptr;
			render_target_view_ = nullptr;
			context_->Flush();

			CreateDeviceResources();
			CreateWindowSizeDependentResources();

			if (device_notify_ != nullptr) { device_notify_->OnDeviceRestored(); }
		}

		void RegisterDeviceNotify(IDeviceNotify* deviceNotify) {
			device_notify_ = deviceNotify;
		}

		// Device Accessors.
		winrt::Windows::Foundation::Size GetOutputSize() const { return output_size_; }
		winrt::Windows::Foundation::Size GetLogicalSize() const { return logical_size_; }
		winrt::Windows::Foundation::Size GetRenderTargetSize() const { return render_target_size_; }
		float GetDpi() const { return dpi_; }

		// D3D Accessors.
		//winrt::com_ptr<ID3D11Device> const& GetD3DDevice() const { return mdevice_; }
		ID3D11DeviceContext3* GetD3DDeviceContext() const { return context_.get(); }
		IDXGISwapChain1* GetSwapChain() const { return swap_chain_.get(); }
		D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const { return feature_level_; }
		ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return render_target_view_.get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return depth_stencil_view_.get(); }
		D3D11_VIEWPORT          GetScreenViewport() const { return screen_viewport_; }

	private:
		// Device creation and initialization inner methods
		void CreateDeviceIndependentResources() {}
		void CreateDeviceResources() {}
		void CreateWindowSizeDependentResources() {}


		// Direct3D objects.
		winrt::com_ptr<ID3D11Device2> mdevice_;
		winrt::com_ptr<ID3D11DeviceContext3> context_;
		winrt::com_ptr<IDXGISwapChain1> swap_chain_;

		// Direct3D rendering objects. Required for 3D.
		winrt::com_ptr<ID3D11RenderTargetView> render_target_view_;
		winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view_;
		D3D11_VIEWPORT screen_viewport_;

		// Cached reference to the Window.
		winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> window_{ nullptr };


		// Cached device properties.
		D3D_FEATURE_LEVEL feature_level_;
		winrt::Windows::Foundation::Size render_target_size_;
		winrt::Windows::Foundation::Size output_size_;
		winrt::Windows::Foundation::Size logical_size_;
		float dpi_;

		// The IDeviceNotify can be held directly as it owns the DeviceResources.
		IDeviceNotify* device_notify_;
	};