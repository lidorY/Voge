#pragma once
#include "pch.h"

#include <wincodec.h>
#include "BasicReaderWriter.h"
// A simple loader class that provides support for loading shaders
// from files on disk. Provides synchronous and asynchronous methods.
class Loader {
public:

    Loader(
        _In_ winrt::com_ptr<ID3D11Device3> d3dDevice,
        _In_opt_ IWICImagingFactory2* wicFactory = nullptr
    );

    // Input Layout loader
    void LoadShader(
        _In_ winrt::hstring const& filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        _In_ winrt::hstring filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    // Vertex shader loader
    void LoadShader(
        _In_ winrt::hstring const& filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        _In_ winrt::hstring filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    );

    // Pixel shader loader
    void LoadShader(
        _In_ winrt::hstring const& filename,
        _Out_ ID3D11PixelShader** shader
    );

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        _In_ winrt::hstring filename,
        _Out_ ID3D11PixelShader** shader
    );


private:
    winrt::com_ptr<ID3D11Device> m_d3dDevice;
    winrt::com_ptr<IWICImagingFactory2> m_wicFactory;
    BasicReaderWriter m_basicReaderWriter;

}