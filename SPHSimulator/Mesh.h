#pragma once
#include "Resource.h"

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
	void SetVertexData(void* data, UINT count);
	void SetIndexData(void* data, UINT count);
	void CreateIndexBuffer(void* data, UINT count, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT);

	void SetIndexSize(UINT index) { _indexes = 0; }
	void BindBuffer();
	void Render();
	void RenderInstanced(InstancingBuffer* instances);
	void RenderIndexedInstancedIndirect(InstancingBuffer* instances, IndirectBuffer* indirect);

private:
	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;
	UINT _indexes;
	UINT _vertexes;
};
