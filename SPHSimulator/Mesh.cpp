#define NOMINMAX

#include "pch.h"
#include "Mesh.h"
#include "Graphics.h"
#include <fstream>
#include <sstream>
#include "InstancingBuffer.h"
#include "IndirectBuffer.h"
#include <limits>

Mesh::Mesh()
	:Resource(RESOURCE_TYPE::MESH)
	, _indexes(0)
	, sdf{}
	, projection{}
	, indexes{}
	, vertexes{}
	, _vertexes{}
{

}

Mesh::~Mesh()
{
}

// find distance x0 is from segment x1-x2
static float point_segment_distance(const Vector3& x0, const Vector3& x1, const Vector3& x2, Vector3& proj) {
	Vector3 dx(x2 - x1);
	float m2 = dx.LengthSquared();
	// find parameter value of closest point on segment
	float s12 = float((x2 - x0).Dot(dx) / m2);
	if (s12 < 0) {
		s12 = 0;
	}
	else if (s12 > 1) {
		s12 = 1;
	}
	// and find the distance

	proj = s12 * x1 + (1 - s12) * x2;
	return (x0 - proj).Length();
}

// find distance x0 is from triangle x1-x2-x3
static float point_triangle_distance(const Vector3& x0, const Vector3& x1, const Vector3& x2, const Vector3& x3, Vector3& proj) {
	// first find barycentric coordinates of closest point on infinite plane
	Vector3 x13(x1 - x3), x23(x2 - x3), x03(x0 - x3);
	float m13 = x13.LengthSquared(), m23 = x23.LengthSquared(), d = x13.Dot(x23);
	float invdet = 1.f / std::max(m13 * m23 - d * d, 1e-30f);
	float a = x13.Dot(x03), b = x23.Dot(x03);
	// the barycentric coordinates themselves
	float w23 = invdet * (m23 * a - d * b);
	float w31 = invdet * (m13 * b - d * a);
	float w12 = 1 - w23 - w31;
	if (w23 >= 0 && w31 >= 0 && w12 >= 0) { // if we're inside the triangle
		proj = w23 * x1 + w31 * x2 + w12 * x3;
		return (x0 - proj).Length();
	}
	else { // we have to clamp to one of the edges
		Vector3 proj1;
		Vector3 proj2;
		Vector3 proj3;

		float d1 = point_segment_distance(x0, x1, x2, proj1);
		float d2 = point_segment_distance(x0, x1, x3, proj2);
		float d3 = point_segment_distance(x0, x2, x3, proj3);
		if (d1 <= d2 && d1 <= d3) {
			proj = proj1;
			return d1;
		}
		else if(d2 <= d1 && d2 <= d3){
			proj = proj2;
			return d2;
		}
		else{
			proj = proj3;
			return d3;
		}
	}
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

		CreateVertexBuffer(vertexes.data(), (UINT)vertexes.size());
		CreateIndexBuffer(indexes.data(), (UINT)indexes.size());
		_vertexes = (UINT)vertexes.size();
	}

	return S_OK;
}

void Mesh::CreateVertexBuffer(vector<Vertex> vertexVector, D3D11_USAGE usage)
{
	vertexes = vertexVector;
	// 버텍스 버퍼
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(Vertex) * vertexVector.size();
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = usage;
	if (usage == D3D11_USAGE_DYNAMIC)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = vertexVector.data();
	DEVICE->CreateBuffer(&desc, &subData, _vertexBuffer.GetAddressOf());
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

void Mesh::CreateIndexBuffer(vector<UINT> indexVector, D3D11_USAGE usage)
{
	indexes = indexVector;
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(UINT) * indexVector.size();
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	desc.Usage = usage;
	if (usage == D3D11_USAGE_DYNAMIC)
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = indexVector.data();
	DEVICE->CreateBuffer(&desc, &subData, _indexBuffer.GetAddressOf());
	_indexes = indexVector.size();
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

	for (int z = int(minV.z / h) - 1; z <= int(maxV.z / h) + 1; ++z) {
		for (int y = int(minV.y / h) - 1; y <= int(maxV.y / h) + 1; ++y) {
			for (int x = int(minV.x / h) - 1; x <= int(maxV.x / h) + 1; ++x) {
				Vector3 center = Vector3(x + 0.5f, y + 0.5f, z + 0.5f) * h;
				Vector3 proj;
				float d = point_triangle_distance(center, p0, p1, p2, proj);
				if (sdf.find(make_tuple(x, y, z)) == sdf.end() || sdf[make_tuple(x, y, z)] > d)
				{
					sdf[make_tuple(x, y, z)] = d;
					projection[make_tuple(x, y, z)] = proj;
				}
			}
		}
	}
}

void Mesh::Voxelize(vector<Vector3>& voxels, float cellSize, const Matrix& srt)
{
	for (unsigned long long i = 0; 3 * i < _indexes; i++)
	{
		Vector3 p0 = Vector3::Transform(vertexes[indexes[(size_t)3 * i]].pos, srt);
		Vector3 p1 = Vector3::Transform(vertexes[indexes[(size_t)3 * i + 1ULL]].pos, srt);
		Vector3 p2 = Vector3::Transform(vertexes[indexes[(size_t)3 * i + 2ULL]].pos, srt);
		rasterizeTriangle(p0, p1, p2, cellSize);
	}

	for (auto& particle : projection)
	{
		voxels.push_back(particle.second);
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
