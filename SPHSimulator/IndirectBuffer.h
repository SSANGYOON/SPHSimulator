#pragma once

struct IndirectArgs {
	UINT VertexCountPerInstance;
	UINT InstanceCount;
	UINT StartVertexLocation;
	UINT StartInstanceLocation;
};

class IndirectBuffer
{
public:
	IndirectBuffer(UINT numElements, const void* initData);
	~IndirectBuffer();

	void BindUAV(UINT slot);
	void ClearUAV();

private:
	ComPtr<ID3D11Buffer> _buffer;

	ComPtr<ID3D11UnorderedAccessView> _UAV;

	UINT SlotNum;
};

