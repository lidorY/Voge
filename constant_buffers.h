#pragma once

#include <d3d11.h>
#include <dxgiformat.h>
#include <DirectXMath.h>

struct PNVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
};

// TODO: change to c++ array?
static D3D11_INPUT_ELEMENT_DESC PNVertexLayout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

struct ConstantBufferNeverChanges {
	DirectX::XMFLOAT4 light_position[4];
	DirectX::XMFLOAT4 light_color;
};

struct ConstantBufferChangeOnResize {
	DirectX::XMFLOAT4X4 projection;
};

struct ConstantBufferChangesEveryFrame {
	DirectX::XMFLOAT4X4 view;
};

struct ConstantBufferChangesEveryPrim {
	DirectX::XMFLOAT4X4 world_matrix;
	DirectX::XMFLOAT4 mesh_color;
	DirectX::XMFLOAT4 diffuse_Color;
	DirectX::XMFLOAT4 specular_Color;
	float specular_power;
};