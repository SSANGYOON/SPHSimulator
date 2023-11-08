#include "pch.h"
#include "InstancingBuffer.h"
#include "Graphics.h"

InstancingBuffer::InstancingBuffer()
{
}

InstancingBuffer::~InstancingBuffer()
{
}

void InstancingBuffer::Init(UINT maxCount)
{
	_maxCount = maxCount;
	const UINT bufferSize = sizeof(Vector3) * maxCount;

	D3D11_BUFFER_DESC desc;

	desc.ByteWidth = bufferSize;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	DEVICE->CreateBuffer(&desc, nullptr, _buffer.GetAddressOf());
}

void InstancingBuffer::Clear()
{
	_data.clear();
}

void InstancingBuffer::AddData(Vector3& params)
{
	_data.push_back(params);
}

void InstancingBuffer::PushData(const vector<Vector3>& data)
{
	_count = data.size();
	Init(_count);

	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, data.data(), data.size() * sizeof(Vector3));
	CONTEXT->Unmap(_buffer.Get(), 0);
}

void InstancingBuffer::PushData()
{
	_count = GetCount();
	if (_count > _maxCount)
		Init(_count);

	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, _data.data(), _data.size() * sizeof(Vector3));
	CONTEXT->Unmap(_buffer.Get(), 0);
}

void InstancingBuffer::SetDataFromBuffer(ComPtr<ID3D11Buffer> _src)
{
	CONTEXT->CopyResource(_buffer.Get(), _src.Get());
}
