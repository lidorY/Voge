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

namespace ScreenRotation
{
	// 0-degree Z-rotation
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}


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

		winrt::com_ptr<IDXGIDevice3> dxgi_device;
		// This conversion doesn't seem to work..
		dxgi_device = mdevice_.as<IDXGIDevice3>();
		winrt::com_ptr<IDXGIAdapter> device_adapter;
		winrt::check_hresult(dxgi_device->GetAdapter(device_adapter.put()));
		winrt::com_ptr<IDXGIFactory2> device_factory;
		winrt::check_hresult(device_adapter->GetParent(IID_PPV_ARGS(&device_factory)));
		winrt::com_ptr<IDXGIAdapter1> previous_default_adapter;
		winrt::check_hresult(device_factory->EnumAdapters1(0, previous_default_adapter.put()));


		DXGI_ADAPTER_DESC previous_desc;
		winrt::check_hresult(previous_default_adapter->GetDesc(&previous_desc));

		// Next, get the information for the current default adapter.
		winrt::com_ptr<IDXGIFactory2> current_factory;
		winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(&current_factory)));
		winrt::com_ptr<IDXGIAdapter1> current_default_adapter;
		winrt::check_hresult(current_factory->EnumAdapters1(0, current_default_adapter.put()));


		DXGI_ADAPTER_DESC current_desc;
		winrt::check_hresult(current_default_adapter->GetDesc(&current_desc));


		if (previous_desc.AdapterLuid.LowPart != current_desc.AdapterLuid.LowPart ||
			previous_desc.AdapterLuid.HighPart != current_desc.AdapterLuid.HighPart ||
			FAILED(mdevice_->GetDeviceRemovedReason()))
		{
			// TODO: this implies we need to release the refernces
			// before we call HandleDeviceLost. Is this true or just error in code?

			// Release references to resources related to the old device.
			dxgi_device = nullptr;
			device_adapter = nullptr;
			device_factory = nullptr;
			previous_default_adapter = nullptr;

			// Create a new device and swap chain.
			HandleDeviceLost();
		}
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
	void CreateDeviceIndependentResources() {
			

	}

	void CreateDeviceResources() {
		// This flag adds support for surfaces with a different color channel ordering
		// than the API default. It is required for compatibility with Direct2D.
		UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		//This array defines the set of DirectX hardware feature levels this app will support.
		D3D_FEATURE_LEVEL feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		// Create the Direct3D 11 API device object and a corresponding context.
		winrt::com_ptr<ID3D11Device> device;
		winrt::com_ptr<ID3D11DeviceContext> context;

		HRESULT hr = D3D11CreateDevice(
			nullptr,                    // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
			0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			creation_flags,             // Set debug and Direct2D compatibility flags.
			feature_levels,             // List of feature levels this app can support.
			ARRAYSIZE(feature_levels),  // Size of the list above.
			D3D11_SDK_VERSION,          // Always set D3D11_SDK_VERSION for Windows Runtime apps.
			device.put(),               // Returns the Direct3D device created.
			&feature_level_,            // Returns feature level of device created.
			context.put()               // Returns the device immediate context.
		);

		if (FAILED(hr)) {
			// Not proceeing in case we failed creating the device..
			// TODO: Should I enable WARP device feature?
			std::terminate();
		}

		// Store pointers to the Direct3D 11.1 API device and immediate context.
		mdevice_ = device.as<ID3D11Device3>();
		context_ = context.as<ID3D11DeviceContext3>();
	}

	float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}
		
	float ConvertPixelsToDips(float pixels)
	{
		static const float dipsPerInch = 96.0f;
		return pixels * dipsPerInch / winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().LogicalDpi(); // Do not round.
	}

	void CreateWindowSizeDependentResources() {
		// Clear the previous window size specific context.
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		context_->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		render_target_view_ = nullptr;
		depth_stencil_view_ = nullptr;
		context_->Flush();

		// Calculate the necessary swap chain and render target size in pixels.
		output_size_.Width =  ConvertDipsToPixels(logical_size_.Width, dpi_);
		output_size_.Height = ConvertDipsToPixels(logical_size_.Height, dpi_);

		// Prevent zero size DirectX content from being created.
		output_size_.Width = max(output_size_.Width, 1);
		output_size_.Height = max(output_size_.Height, 1);

		// TODO: this is needed only when we support both landscape and portrait
		// irrelevant here, so I might get rid of that..
		render_target_size_.Width = output_size_.Width;
		render_target_size_.Height = output_size_.Height;

		if (swap_chain_ != nullptr) {
			// If the swap chain already exists, resize it.
			HRESULT hr = swap_chain_->ResizeBuffers(
				2, // Double-buffered swap chain.
				static_cast<UINT>(render_target_size_.Width),
				static_cast<UINT>(render_target_size_.Height),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
			);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
				// If the device was removed for any reason,
				// a new device and swap chain will need to be created.
				HandleDeviceLost();
				// Everything is set up now. Do not continue execution of this method.
				//HandleDeviceLost will reenter this method 
				// and correctly set up the new device.
				return;
			}
			else {
				winrt::check_hresult(hr);
			}
		}
		else {
			// For non-existent chain:
			// create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			// Match the size of the window.
			swapChainDesc.Width = static_cast<UINT>(render_target_size_.Width); 
			swapChainDesc.Height = static_cast<UINT>(render_target_size_.Height);
			// This is the most common swap chain format.
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; 
			swapChainDesc.Stereo = false;
			// Don't use multi-sampling.
			swapChainDesc.SampleDesc.Count = 1; 
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			// Use double-buffering to minimize latency.
			swapChainDesc.BufferCount = 2; 
			// All Windows Store apps must use this SwapEffect.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; 
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			// This sequence obtains the DXGI factory 
			// that was used to create the Direct3D device above.
			winrt::com_ptr<IDXGIDevice3> dxgiDevice;
			mdevice_->QueryInterface(__uuidof(IDXGIDevice3), (void**)&dxgiDevice);
			winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
			winrt::check_hresult(
				dxgiDevice->GetAdapter(dxgiAdapter.put())
			);
			winrt::com_ptr<IDXGIFactory3> dxgiFactory;
			winrt::check_hresult(
				dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

			winrt::check_hresult(
				dxgiFactory->CreateSwapChainForCoreWindow(
					mdevice_.get(),
					winrt::get_unknown(window_.get()),
					&swapChainDesc,
					nullptr,
					swap_chain_.put()
				)
			);

			// Ensure that DXGI does not queue more than one frame at a time. 
			// This both reduces latency and
			// ensures that the application will only render after each VSync,
			// minimizing power consumption.
			winrt::check_hresult(
				dxgiDevice->SetMaximumFrameLatency(1)
			);
		}

		orientation_transform3d_ = ScreenRotation::Rotation0;
		winrt::check_hresult(
			swap_chain_->SetRotation(DXGI_MODE_ROTATION_IDENTITY)
		);

		// Create a render target view of the swap chain back buffer.
		winrt::com_ptr<ID3D11Texture2D> backBuffer = winrt::capture<ID3D11Texture2D>(swap_chain_,
			&IDXGISwapChain1::GetBuffer, 0);
		winrt::check_hresult(
			mdevice_->CreateRenderTargetView(
				backBuffer.get(),
				nullptr,
				render_target_view_.put()
			)
		);

		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			static_cast<UINT>(render_target_size_.Width),
			static_cast<UINT>(render_target_size_.Height),
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);
		winrt::com_ptr<ID3D11Texture2D> depthStencil;
		winrt::check_hresult(
			mdevice_->CreateTexture2D(
				&depthStencilDesc,
				nullptr,
				depthStencil.put()
			)
		);
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		winrt::check_hresult(
			mdevice_->CreateDepthStencilView(
				depthStencil.get(),
				&depthStencilViewDesc,
				depth_stencil_view_.put()
			)
		);

		// Set the 3D rendering viewport to target the entire window.
		screen_viewport_ = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			render_target_size_.Width,
			render_target_size_.Height
		);
		context_->RSSetViewports(1, &screen_viewport_);
	}


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

	DirectX::XMFLOAT4X4 orientation_transform3d_;
};