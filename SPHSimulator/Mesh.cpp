#include "pch.h"
#include "Mesh.h"
#include "Graphics.h"

Mesh::Mesh()
	:Resource(RESOURCE_TYPE::MESH)
	, _indexes(0)
{
}

Mesh::~Mesh()
{
}

HRESULT Mesh::Load(const std::wstring& path, bool stockObject)
{
	return E_NOTIMPL;
}

void Mesh::CreateVertexBuffer(void* data, UINT count, D3D11_USAGE usage)
{
	// 버텍스 버퍼
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(Vertex) * count;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = usage;
	if (usage == D3D11_USAGE_DYNAMIC)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = data;
	DEVICE->CreateBuffer(&desc, &subData, _vertexBuffer.GetAddressOf());
}

void Mesh::SetVertexData(void* data, UINT count)
{
	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, data, sizeof(Vertex) * count);
	CONTEXT->Unmap(_vertexBuffer.Get(), 0);
}

void Mesh::CreateIndexBuffer(void* data, UINT count, D3D11_USAGE usage)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(UINT) * count;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	desc.Usage = usage;
	if (usage == D3D11_USAGE_DYNAMIC)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		desc.CPUAccessFlags = 0; 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = data;
	DEVICE->CreateBuffer(&desc, &subData, _indexBuffer.GetAddressOf());
	_indexes = count;
}

void Mesh::SetIndexData(void* data, UINT count)
{
	D3D11_MAPPED_SUBRESOURCE sub = {};
	CONTEXT->Map(_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	memcpy(sub.pData, data, sizeof(UINT) * count);
	CONTEXT->Unmap(_indexBuffer.Get(), 0);
	_indexes = count;
}

void Mesh::BindBuffer()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	CONTEXT->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
	CONTEXT->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void Mesh::Render()
{
	CONTEXT->DrawIndexed(_indexes, 0, 0);
}