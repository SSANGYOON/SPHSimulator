#pragma once
#include "Resource.h"
#include <map>

struct Vertex
{
	Vector3 pos;
	Vector2 uv;
	Vector3 normal;
	Vector4 Color;
};

class InstancingBuffer;
class IndirectBuffer;
class Mesh : public Resource
{
public:
	Mesh();
	virtual ~Mesh();

	virtual HRESULT Load(const std::wstring& path, bool stockObject = true) override;

public:
	void CreateVertexBuffer(void* data, UINT count, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void CreateVertexBuffer(vector<Vertex> vertexVector, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void SetVertexData(void* data, UINT count);
	void SetIndexData(void* data, UINT count);
	void CreateIndexBuffer(vector<UINT> indexVector, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void CreateIndexBuffer(void* data, UINT count, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);
	void rasterizeTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, float h);
	void Voxelize(vector<Vector3>& voxels, float cellSize, const Matrix& srt);

	void SetIndexSize(UINT index) { _indexes = 0; }
	void BindBuffer();
	void Render();
	void RenderInstanced(InstancingBuffer* instances);
	void RenderIndexedInstancedIndirect(InstancingBuffer* instances, IndirectBuffer* indirect);

	const vector<Vertex>& GetVertexes() const { return vertexes; }
	const vector<UINT>& GetIndexes() const { return indexes; }

private:
	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;
	vector<Vertex> vertexes;
	vector<UINT> indexes;
	map<tuple<int, int, int>, float> sdf;
	map<tuple<int, int, int>, Vector3> projection;
	UINT _indexes;
	UINT _vertexes;
};
