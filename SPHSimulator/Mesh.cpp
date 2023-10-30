#include "pch.h"
#include "Mesh.h"
#include "Graphics.h"
#include <fstream>
#include <sstream>
#include "InstancingBuffer.h"
#include "IndirectBuffer.h"

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
	fstream fs;
	fs.open(path, ios_base::in);

	if (!fs.is_open())
	{
		return S_FALSE;
	}
	else
	{
		string line;
		vector<Vector3> positions;
		vector<Vector3> normals;
		vector<Vector2> uvs;
		vector<Vertex> vertexes;
		while (getline(fs, line))
		{
			std::stringstream ss;
			ss << line;
			
			std::string label;
			ss >> label;
			if (label == "v") {
				//construct a point from the line
				Vector3 point;
				ss >> point.x;
				ss >> point.y;
				ss >> point.z;

				//push the point to the point list
				positions.push_back(point);
			}
			if (label == "vt") {
				Vector2 normal;
				ss >> normal.x;
				ss >> normal.y;

				//push the normal to the normals list
				uvs.push_back(normal);
			}
			if (label == "vn") {
				Vector3 normal;
				ss >> normal.x;
				ss >> normal.y;
				ss >> normal.z;

				//push the normal to the normals list
				normals.push_back(normal);
			}

			if (label == "f") {

				for (int i = 0; i < 3; i++)
				{
					std::string s;
					size_t pos;
					std::string v;

					//search face for / delimeter

					int a, b, c;

					ss >> s;
					pos = s.find("/");
					v = s.substr(0, pos);
					a = std::stoi(v) - 1;

					s.erase(0, pos + 1);

					pos = s.find("/");
					v = s.substr(0, pos);
					b = std::stoi(v) - 1;

					s.erase(0, pos + 1);

					c = std::stoi(s) - 1;
					Vertex vx;
					vx.pos = positions[a];
					vx.uv = uvs[b];
					vx.normal = normals[c];

					vertexes.push_back(vx);
				}
			}

			
		}
		fs.close();

		CreateVertexBuffer(vertexes.data(), vertexes.size());
		_vertexes = vertexes.size();
	}

	return S_OK;
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
	if(_indexBuffer)
		CONTEXT->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void Mesh::Render()
{
	if (_indexBuffer)
		CONTEXT->DrawIndexed(_indexes, 0, 0);
	else
		CONTEXT->Draw(_vertexes,0);
}

void Mesh::RenderInstanced(InstancingBuffer* instances)
{
	if (!_indexBuffer)
	{

		UINT stride[] = { sizeof(Vertex), sizeof(Matrix) };
		UINT offset[] = { 0, 0 };

		ID3D11Buffer* views[] = { _vertexBuffer.Get(), instances->GetBuffer() };
		CONTEXT->IASetVertexBuffers(0, 2, views, stride, offset);
		CONTEXT->DrawInstanced(_vertexes, instances->GetCount(), 0, 0);
	}
}

void Mesh::RenderIndexedInstancedIndirect(InstancingBuffer* instances, IndirectBuffer* indirect)
{
	if (_indexBuffer)
	{
		UINT stride[] = { sizeof(Vertex), sizeof(Vector3) };
		UINT offset[] = { 0, 0 };

		ID3D11Buffer* views[] = { _vertexBuffer.Get(), instances->GetBuffer() };
		CONTEXT->IASetVertexBuffers(0, 2, views, stride, offset);
		CONTEXT->DrawIndexedInstancedIndirect(indirect->GetBuffer().Get(), 0);
	}
}
