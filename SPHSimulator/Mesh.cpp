#define NOMINMAX

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
	, voxelset{}
	, indexes{}
	, vertexes{}
{

}

Mesh::~Mesh()
{
}

HRESULT Mesh::Load(const std::wstring& path, bool stockObject)
{
	std::filesystem::path parentPath = std::filesystem::current_path().parent_path();
	wstring fullpath = parentPath.wstring() + L"\\Resources\\" + path;

	fstream fs;
	fs.open(fullpath, ios_base::in);

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

		int s = 0;

		UINT ind = 0;
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
			if (label == "s") {
				ss >> s;
			}

			if (label == "f") {

				if (s == 2) {
					for (int i = 0; i < 4; i++)
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

					indexes.push_back(ind * 4);
					indexes.push_back(ind * 4 + 1);
					indexes.push_back(ind * 4 + 2);
					indexes.push_back(ind * 4 + 0);
					indexes.push_back(ind * 4 + 2);
					indexes.push_back(ind * 4 + 3);

					ind++;
				}
				else
				{
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

					indexes.push_back(ind * 3);
					indexes.push_back(ind * 3 + 1);
					indexes.push_back(ind * 3 + 2);

					ind++;
				}
			}	
		}
		fs.close();

		CreateVertexBuffer(vertexes.data(), vertexes.size());
		CreateIndexBuffer(indexes.data(), indexes.size());
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
		desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = data;
	DEVICE->CreateBuffer(&desc, &subData, _indexBuffer.GetAddressOf());
	_indexes = count;
}

void Mesh::rasterizeTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, float h)
{
	Vector3 minV = Vector3::Min(Vector3::Min(p0, p1),p2);
	Vector3 maxV = Vector3::Max(Vector3::Max(p0, p1), p2);

	Vector3 e0 = p1 - p0;
	Vector3 e1 = p2 - p0;
	Vector3 normal = e0.Cross(e1);

	normal.Normalize();

	for (int z = int(minV.z / (h * 0.5f)) - 1; z <= int(maxV.z / (h * 0.5f)) + 1; ++z) {
		for (int y = int(minV.y / (h * 0.5f)) - 1; y <= int(maxV.y / (h * 0.5f)) + 1; ++y) {
			for (int x = int(minV.x / (h * 0.5f)) - 1; x <= int(maxV.x / (h * 0.5f)) + 1; ++x) {
				Vector3 center = Vector3(x + 0.5f, y + 0.5f, z + 0.5f) * (h * 0.5f);
				float d = (center - p0).Dot(normal);

				// check if voxel intersects triangle plane
				if (std::abs(d) < pow(2.f, 0.5f) * (h * 0.5f)) {

					// check if projected voxel center lies within triangle
					Vector3 p = center - d * normal;
					float twoArea = e0.Cross(e1).Length();
					float s = (p - p0).Cross(e1).Length();
					float t = (p - p0).Cross(e0).Length();

					if (twoArea  >= s + t)
						voxelset.insert(make_tuple(x,y,z));
				}
			}
		}
	}
}

void Mesh::Voxelize(vector<Vector3>& voxels, float cellSize, const Matrix& srt)
{
	for (int i = 0; i < _indexes; i += 3)
	{
		Vector3 p0 = Vector3::Transform(vertexes[i].pos, srt);
		Vector3 p1 = Vector3::Transform(vertexes[i + 1].pos, srt);
		Vector3 p2 = Vector3::Transform(vertexes[i + 2].pos, srt);
		rasterizeTriangle(p0, p1, p2, cellSize);
	}

	for (auto& voxel : voxelset)
	{
		voxels.push_back(Vector3((float)get<0>(voxel) + 0.5f, (float)get<1>(voxel) + 0.5f, (float)get<2>(voxel) + 0.5f) * (cellSize * 0.5f));
	}
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
		UINT stride[] = { sizeof(Vertex), sizeof(Vector3) };
		UINT offset[] = { 0, 0 };

		ID3D11Buffer* views[] = { _vertexBuffer.Get(), instances->GetBuffer() };
		CONTEXT->IASetVertexBuffers(0, 2, views, stride, offset);
		CONTEXT->DrawInstanced(_vertexes, instances->_count, 0, 0);
	}
	else
	{
		UINT stride[] = { sizeof(Vertex), sizeof(Vector3) };
		UINT offset[] = { 0, 0 };

		ID3D11Buffer* views[] = { _vertexBuffer.Get(), instances->GetBuffer() };
		CONTEXT->IASetVertexBuffers(0, 2, views, stride, offset);
		CONTEXT->DrawIndexedInstanced(_indexes, instances->_count, 0, 0, 0);
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
