#include "pch.h"
#include "StructuredBuffer.h"
#include "Graphics.h"

StructuredBuffer::StructuredBuffer()
	: _SRV(nullptr)
	, _size(0)
	, _stride(0)
	, _SRVSlot(0)
	, _UAVSlot(0)
	, _readBuffer(nullptr)
	, _asUAV(false)
	, _readable(false)
	, desc{}
{
}

StructuredBuffer::~StructuredBuffer()
{
}

bool StructuredBuffer::Create(UINT size, UINT stride, void* data, bool asUAV, bool _readable)
{
	_asUAV = asUAV;
	_size = size;
	_stride = stride;


	setDiscription();
	createBuffer(data);
	createView();

	if (_readable)
		createReadBuffer();

	return true;
}

void StructuredBuffer::SetData(void* data, UINT bufferCount)
{
	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, data, _size * bufferCount);
	_stride = bufferCount;
	CONTEXT->Unmap(_buffer.Get(), 0);
}

void StructuredBuffer::GetData(void* data)
{
	CONTEXT->CopyResource(_readBuffer.Get(), _buffer.Get());

	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_readBuffer.Get(), 0, D3D11_MAP_READ, 0, &sub);
	memcpy(data, sub.pData, _size * _stride);
	CONTEXT->Unmap(_readBuffer.Get(), 0);
}

void StructuredBuffer::BindSRV(UINT slot)
{
	_SRVSlot = slot;

	CONTEXT->VSSetShaderResources(slot, 1, _SRV.GetAddressOf());
	CONTEXT->GSSetShaderResources(slot, 1, _SRV.GetAddressOf());
	CONTEXT->PSSetShaderResources(slot, 1, _SRV.GetAddressOf());
}

void StructuredBuffer::BindUAV(UINT slot)
{
	_UAVSlot = slot;
	UINT i = -1;
	CONTEXT->CSSetUnorderedAccessViews(slot, 1, _UAV.GetAddressOf(), &i);
}

void StructuredBuffer::Clear()
{
	ID3D11ShaderResourceView* srv = nullptr;
	CONTEXT->VSSetShaderResources(_SRVSlot, 1, &srv);
	CONTEXT->GSSetShaderResources(_SRVSlot, 1, &srv);
	CONTEXT->PSSetShaderResources(_SRVSlot, 1, &srv);

	ID3D11UnorderedAccessView* uav = nullptr;
	UINT i = -1;
	CONTEXT->CSSetUnorderedAccessViews(_UAVSlot, 1, &uav, &i);
}

void StructuredBuffer::setDiscription()
{
	desc = {};
	desc.ByteWidth = _size * _stride;
	desc.StructureByteStride = _size;

	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;

	if (_asUAV)
		desc.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;

	desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
}

bool StructuredBuffer::createBuffer(void* data)
{
	if (data)
	{
		D3D11_SUBRESOURCE_DATA tSub = {};
		tSub.pSysMem = data;

		if (FAILED(DEVICE->CreateBuffer(&desc, &tSub, _buffer.GetAddressOf())))
			return false;
	}
	else
	{
		if (FAILED(DEVICE->CreateBuffer(&desc, nullptr, _buffer.GetAddressOf())))
			return false;
	}

	return true;
}

bool StructuredBuffer::createView()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.BufferEx.NumElements = _stride;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFEREX;

	if (FAILED(DEVICE->CreateShaderResourceView(_buffer.Get(), &srvDesc, _SRV.GetAddressOf())))
		return false;

	if (_asUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Buffer.NumElements = _stride;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

		if (FAILED(DEVICE->CreateUnorderedAccessView(_buffer.Get(), &uavDesc, _UAV.GetAddressOf())))
			return false;
	}

	return true;
}

bool StructuredBuffer::createReadBuffer()
{
	D3D11_BUFFER_DESC rDesc = { desc };
	rDesc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	rDesc.BindFlags = 0;
	rDesc.Usage = D3D11_USAGE_STAGING;
	rDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	if (FAILED(DEVICE->CreateBuffer(&rDesc, nullptr, _readBuffer.GetAddressOf())))
		return false;

	return true;
}