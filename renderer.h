#pragma once
#include <memory>

#include "device_resources.h"

class Renderer :public std::enable_shared_from_this<Renderer> {
public:
	void Render();

	void CreateDeviceDependentResources() {}
	void CreateWindowSizeDependentResources() {
		auto context = device_resources_->GetD3DDeviceContext();
		auto render_target_size = device_resources_->GetRenderTargetSize();

		auto bounds = device_resources_->GetLogicalSize();


	}
	void ReleaseDeviceDependentResources() {}

#if defined(DEBUG) || defined(_DEBUG)
	void ReportLiveObjects() {
		winrt::com_ptr<ID3D11Device3> device;
		device = device_resources_->GetD3DDevice();
		auto debug_layer = device.try_as<ID3D11Debug>();
		if (debug_layer) {
			winrt::check_hresult(
				debug_layer->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL)
			);
		}
	}
#endif

	Renderer(std::shared_ptr<DeviceResources> const& dr) :
	device_resources_(dr)
	{
		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();
	}


private:
	std::shared_ptr<DeviceResources> device_resources_;

	// constant Buffer
	winrt::com_ptr<ID3D11Buffer> never_changing_cbuffer_;
	winrt::com_ptr<ID3D11Buffer> resize_changing_cbuffer_;
	winrt::com_ptr<ID3D11Buffer> frame_changing_cbuffer_;
	winrt::com_ptr<ID3D11Buffer> primitive_changing_cbuffer_;

	// Shaders
	winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
	winrt::com_ptr<ID3D11PixelShader> pixel_shader_;

	// Input Layout
	winrt::com_ptr<ID3D11InputLayout> vertex_layout_;

};
