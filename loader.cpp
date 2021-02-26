#include "pch.h"
#include "loader.h"

Loader::Loader(
    _In_ winrt::com_ptr<ID3D11Device3> d3dDevice,
    _In_opt_ IWICImagingFactory2* wicFactory
) :
    m_d3dDevice(std::move(d3dDevice))
{
    m_wicFactory.copy_from(wicFactory);
}
