#pragma once
#include <memory>
#include <winrt/Windows.Foundation.h>

#include "device_resources.h"
#include "constant_buffers.h"
#include "game.h"
#include "loader.h"

class Renderer : public std::enable_shared_from_this<Renderer> {
public:
	void Render() {
		auto context{ device_resources_->GetD3DDeviceContext() };

		ID3D11RenderTargetView* const targets[1] = {
			device_resources_->GetBackBufferRenderTargetView()
		};

		context->OMSetRenderTargets(1, targets, device_resources_->GetDepthStencilView());
		context->ClearDepthStencilView(device_resources_->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH, 1.f, 0);

		const float clear_color[4] = {
			0.5f, 0.5f, 0.8f, 1.f
		};

		context->ClearRenderTargetView(
			device_resources_->GetBackBufferRenderTargetView(), 
			clear_color);
	}

	void CreateDeviceDependentResources() {}
	void CreateWindowSizeDependentResources() {
		auto context = device_resources_->GetD3DDeviceContext();
		auto render_target_size = device_resources_->GetRenderTargetSize();

		auto bounds = device_resources_->GetLogicalSize();

		if (game_ != nullptr)
		{
			XMFLOAT4X4 orientation = device_resources_->GetOrientationTransform3D();

			ConstantBufferChangeOnResize change_on_resize;
			XMStoreFloat4x4(
				&change_on_resize.projection,
				XMMatrixMultiply(
					XMMatrixTranspose(game_->GameCamera().Projection()),
					XMMatrixTranspose(XMLoadFloat4x4(&orientation))
				)
			);

			context->UpdateSubresource(
				resize_changing_cbuffer_.get(),
				0,
				nullptr,
				&change_on_resize,
				0,
				0
			);
		}
	}

	void ReleaseDeviceDependentResources() {}

	winrt::Windows::Foundation::IAsyncAction CreateGameDeviceResourcesAsync(_In_
	std::shared_ptr<Game> game) {
		auto lifetime = shared_from_this();

		// Create the device dependent game resources.
		// Only the d3dDevice is used in this method. It is expected
		// to not run on the same thread as the GameRenderer was created.
		// Create methods on the d3dDevice are free-threaded and are safe while any methods
		// in the d3dContext should only be used on a single thread and handled
		// in the FinalizeCreateGameDeviceResources method.

		auto device = device_resources_->GetD3DDevice();

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		// Create the constant buffers.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		// TODO: Why all this "math" here..?
		bd.ByteWidth = (sizeof(ConstantBufferNeverChanges) + 15) / 16 * 16;
		never_changing_cbuffer_ = nullptr;
		winrt::check_hresult(
			device->CreateBuffer(&bd, nullptr, never_changing_cbuffer_.put())
		);

		bd.ByteWidth = (sizeof(ConstantBufferChangeOnResize) + 15) / 16 * 16;
		resize_changing_cbuffer_ = nullptr;
		winrt::check_hresult(
			device->CreateBuffer(&bd, nullptr, resize_changing_cbuffer_.put())
		);

		bd.ByteWidth = (sizeof(ConstantBufferChangesEveryFrame) + 15) / 16 * 16;
		frame_changing_cbuffer_ = nullptr;
		winrt::check_hresult(
			device->CreateBuffer(&bd, nullptr, frame_changing_cbuffer_.put())
		);

		bd.ByteWidth = (sizeof(ConstantBufferChangesEveryPrim) + 15) / 16 * 16;
		primitive_changing_cbuffer_ = nullptr;
		winrt::check_hresult(
			device->CreateBuffer(&bd, nullptr, primitive_changing_cbuffer_.put())
		);

		// Setting up sampler 
		// TODO: currently i'm not supporting Textures...

		// Start the async tasks to load the shaders and textures.
		Loader loader{ device };

		std::vector<winrt::Windows::Foundation::IAsyncAction> tasks;
		
		uint32_t numElements = ARRAYSIZE(PNVertexLayout);
		
		tasks.push_back(loader.LoadShaderAsync(
			L"VertexShader.cso",
			PNVertexLayout, 
			numElements, 
			vertex_shader_.put(),
			vertex_layout_.put()));

		tasks.push_back(loader.LoadShaderAsync(
			L"PixelShader.cso", 
			pixel_shader_.put()));

		// Wait for all the tasks to complete.
		for (auto&& task : tasks) {
			co_await task;
		}
	}

	void FinalizeCreateGameDeviceResources() {
		auto d3dDevice = device_resources_->GetD3DDevice();
		ConstantBufferNeverChanges constantBufferNeverChanges;

		constantBufferNeverChanges.light_position[0] = XMFLOAT4(3.5f, 2.5f, 5.5f, 1.0f);
		constantBufferNeverChanges.light_position[1] = XMFLOAT4(3.5f, 2.5f, -5.5f, 1.0f);
		constantBufferNeverChanges.light_position[2] = XMFLOAT4(-3.5f, 2.5f, -5.5f, 1.0f);
		constantBufferNeverChanges.light_position[3] = XMFLOAT4(3.5f, 2.5f, 5.5f, 1.0f);
		constantBufferNeverChanges.light_color = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);

		device_resources_->GetD3DDeviceContext()->UpdateSubresource(
			never_changing_cbuffer_.get(),
			0,
			nullptr,
			&constantBufferNeverChanges,
			0,
			0
		);

		// define Meshes and materials here

		// Ensure that the camera has been initialized with the right Projection
		// matrix. The camera is not created at the time the first window resize event
		// occurs.
		auto renderTargetSize = device_resources_->GetRenderTargetSize();
		game_->GameCamera().SetProjParams(
			XM_PI/2,
			renderTargetSize.Width / renderTargetSize.Height,
			0.01f,
			100.f
		);

		// Make sure that Projection matrix has been set in the constant buffer
		// now that all the resources are loaded.
		// We are handling screen rotations directly to eliminate an unaligned
		// fullscreen copy. As a result, it is necessary to post-multiply the rotationTransform3D
		// matrix to the camera projection matrix.
		// The matrices are transposed due to the shader code expecting the matrices in the opposite
		// row/column order from the DirectX math library.
		auto orientation = device_resources_->GetOrientationTransform3D();

		ConstantBufferChangeOnResize changesOnResize;
		XMStoreFloat4x4(
			&changesOnResize.projection,
			XMMatrixMultiply(
				XMMatrixTranspose(game_->GameCamera().Projection()),
				XMMatrixTranspose(XMLoadFloat4x4(&orientation))
			)
		);

		device_resources_->GetD3DDeviceContext()->UpdateSubresource(
			resize_changing_cbuffer_.get(),
			0,
			nullptr,
			&changesOnResize,
			0,
			0
		);

	}

#if defined(DEBUG) || defined(_DEBUG)
	void ReportLiveObjects() {
		// auto does not deduce refernces.
		// Thus in order to avoid unnecessary copy we 
		// define it as com_ptr in advance
		//winrt::com_ptr<ID3D11Device3> device;
		winrt::com_ptr<ID3D11Device3> device = device_resources_->GetD3DDevice();
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

	std::shared_ptr<Game> game_;

};
