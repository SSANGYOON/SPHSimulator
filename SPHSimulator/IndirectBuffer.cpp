#include "pch.h"
#include "IndirectBuffer.h"
#include "Graphics.h"

IndirectBuffer::IndirectBuffer(UINT numElements, UINT stride, const void* initData)
    :_buffer{}
    ,SlotNum(-1)
    , _UAV{}
{

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.ByteWidth = numElements * stride;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags =
        D3D11_BIND_UNORDERED_ACCESS; // ComputeShader에서 업데이트 가능
    desc.StructureByteStride = stride;
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // <- IndirectArgs

    if (initData) {
        D3D11_SUBRESOURCE_DATA bufferData;
        ZeroMemory(&bufferData, sizeof(bufferData));
        bufferData.pSysMem = initData;
        ThrowIfFailed(
            DEVICE->CreateBuffer(&desc, &bufferData, _buffer.GetAddressOf()));
    }
    else {
        ThrowIfFailed(DEVICE->CreateBuffer(&desc, NULL, _buffer.GetAddressOf()));
    }

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
    uavDesc.Buffer.NumElements = numElements;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

    ThrowIfFailed(DEVICE->CreateUnorderedAccessView(_buffer.Get(), &uavDesc, _UAV.GetAddressOf()));
}

IndirectBuffer::~IndirectBuffer()
{
}

void IndirectBuffer::BindUAV(UINT slot)
{
    SlotNum = slot;
    UINT i = -1;
    CONTEXT->CSSetUnorderedAccessViews(slot, 1, _UAV.GetAddressOf(), &i);
}

void IndirectBuffer::ClearUAV()
{
    if (SlotNum > -1)
    {
        ID3D11UnorderedAccessView* uav = nullptr;
        UINT i = -1;
        CONTEXT->CSSetUnorderedAccessViews(SlotNum, 1, &uav, &i);
    }
}

