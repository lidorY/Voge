#pragma once
#include <memory>

#include "device_resources.h"

class Renderer :public std::enable_shared_from_this<Renderer> {
public:
	void Render();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void ReleaseDeviceDependentResources();

#if defined(DEBUG) || defined(_DEBUG)
	void ReportLiveObjects();
#endif

	Renderer(std::shared_ptr<DeviceResources> const& dr) :
	device_resources_(dr)
	{}


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
