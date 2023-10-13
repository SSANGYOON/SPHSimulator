#include "pch.h"
#include "ConstantBuffer.h"
#include "Graphics.h"

ConstantBuffer::ConstantBuffer()
	:desc{}
{
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::Init(Constantbuffer_Type type, UINT size)
{
	_type = type;
	desc.ByteWidth = size;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	DEVICE->CreateBuffer(&desc, nullptr, _buffer.GetAddressOf());
}

void ConstantBuffer::SetData(void* Data)
{
	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, Data, desc.ByteWidth);
	CONTEXT->Unmap(_buffer.Get(), 0);
}

void ConstantBuffer::SetPipline(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::VS:
		CONTEXT->VSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	case ShaderStage::HS:
		CONTEXT->HSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	case ShaderStage::DS:
		CONTEXT->DSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	case ShaderStage::GS:
		CONTEXT->GSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	case ShaderStage::PS:
		CONTEXT->PSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	case ShaderStage::CS:
		CONTEXT->CSSetConstantBuffers((UINT)_type, 1, _buffer.GetAddressOf());
		break;
	default:
		break;
	}
}