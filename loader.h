#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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
    ) :
        m_d3dDevice(std::move(d3dDevice))
    {
        m_wicFactory.copy_from(wicFactory);
    }

    // Vertex Shader loader
    void LoadShader(
        _In_ winrt::hstring const& filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    ) {
        std::vector<byte> bytecode{ m_basicReaderWriter.ReadData(filename) };
        winrt::check_hresult(
            m_d3dDevice->CreateVertexShader(
                bytecode.data(),
                bytecode.size(),
                nullptr,
                shader
            )
        );
        SetDebugName(*shader, filename);
        if(layout != nullptr) {
            CreateInputLayout(
                bytecode.data(),
                bytecode.size(),
                layoutDesc,
                layoutDescNumElements,
                layout
            );

            SetDebugName(*layout, filename);
        }
    }

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        _In_ winrt::hstring filename,
        _In_reads_opt_(layoutDescNumElements) D3D11_INPUT_ELEMENT_DESC const layoutDesc[],
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11VertexShader** shader,
        _Out_opt_ ID3D11InputLayout** layout
    ) {
        // This method assumes that the lifetime of input arguments may be shorter
        // than the duration of this operation. In order to ensure accurate results, a
        // copy of all arguments passed by pointer must be made. The method then
        // ensures that the lifetime of the copied data exceeds that of the coroutine.

        // Create copies of the layoutDesc array as well as the SemanticName strings,
        // both of which are pointers to data whose lifetimes may be shorter than that
        // of this method's coroutine.
        std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDescCopy;
        std::vector<std::string> layoutDescSemanticNamesCopy;
        if (layoutDesc != nullptr) {
            layoutDescCopy = { layoutDesc, layoutDesc + layoutDescNumElements };
            layoutDescSemanticNamesCopy.reserve(layoutDescNumElements);
            for (auto && desc : layoutDescCopy)
            {
                desc.SemanticName = layoutDescSemanticNamesCopy.emplace
                 (layoutDescSemanticNamesCopy.end(), desc.SemanticName)->c_str();
            }
        }

        auto bytecode = co_await m_basicReaderWriter.ReadDataAsync(filename);
        winrt::check_hresult(
            m_d3dDevice->CreateVertexShader(
                bytecode.data(),
                bytecode.Length(),
                nullptr,
                shader
            )
        );
        SetDebugName(*shader, filename);
        if (layout != nullptr) {
            CreateInputLayout(
                bytecode.data(),
                bytecode.Length(),
                layoutDesc == nullptr ? nullptr : layoutDescCopy.data(),
                layoutDescNumElements,
                layout
            );
            SetDebugName(*layout, filename);
        }
    }


    // Pixel shader loader
    void LoadShader(
        _In_ winrt::hstring const& filename,
        _Out_ ID3D11PixelShader** shader
    ) {
        std::vector<byte> bytecode{ m_basicReaderWriter.ReadData(filename) };
        winrt::check_hresult(
            m_d3dDevice->CreatePixelShader(
                bytecode.data(),
                bytecode.size(),
                nullptr,
                shader
            )
        );
        SetDebugName(*shader, filename);
    }

    winrt::Windows::Foundation::IAsyncAction LoadShaderAsync(
        _In_ winrt::hstring filename,
        _Out_ ID3D11PixelShader** shader
    ) {
        auto bytecode = co_await m_basicReaderWriter.ReadDataAsync(filename);
        winrt::check_hresult(
            m_d3dDevice->CreatePixelShader(
                bytecode.data(),
                bytecode.Length(),
                nullptr,
                shader
            )
        );

        SetDebugName(*shader, filename);
    }

    template <class DeviceChildType>
    inline void SetDebugName(
        _In_ DeviceChildType* object,
        _In_ winrt::hstring const& name
    )
    {
#if defined(_DEBUG)
        // Only assign debug names in debug builds.
        char nameString[1024];
        int nameStringLength = WideCharToMultiByte(
            CP_ACP,
            0,
            name.c_str(),
            -1,
            nameString,
            1024,
            nullptr,
            nullptr
        );
        if (nameStringLength == 0) {
            char defaultNameString[] = "BasicLoaderObject";
            winrt::check_hresult(
                object->SetPrivateData(
                    WKPDID_D3DDebugObjectName,
                    sizeof(defaultNameString) - 1,
                    defaultNameString
                )
            );
        } 
        else {
            winrt::check_hresult(
                object->SetPrivateData(
                    WKPDID_D3DDebugObjectName,
                    nameStringLength - 1,
                    nameString
                )
            );
        }
#endif
    }

    void CreateInputLayout(
        _In_reads_bytes_(bytecodeSize) const byte* bytecode,
        _In_ uint32_t bytecodeSize,
        _In_reads_opt_(layoutDescNumElements) const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        _In_ uint32_t layoutDescNumElements,
        _Out_ ID3D11InputLayout** layout
    )
    {
        if (layoutDesc == nullptr)
        {
            // If no input layout is specified, use the BasicVertex layout.
            static constexpr D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            winrt::check_hresult(
                m_d3dDevice->CreateInputLayout(
                    basicVertexLayoutDesc,
                    ARRAYSIZE(basicVertexLayoutDesc),
                    bytecode,
                    bytecodeSize,
                    layout
                )
            );
        }
        else
        {
            winrt::check_hresult(
                m_d3dDevice->CreateInputLayout(
                    layoutDesc,
                    layoutDescNumElements,
                    bytecode,
                    bytecodeSize,
                    layout
                )
            );
        }
    }

private:
    winrt::com_ptr<ID3D11Device> m_d3dDevice;
    winrt::com_ptr<IWICImagingFactory2> m_wicFactory;
    BasicReaderWriter m_basicReaderWriter;

};