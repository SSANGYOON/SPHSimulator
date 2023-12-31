#pragma once

struct IndirectArgs {
	UINT IndexCountPerInstance;
	UINT InstanceCount;
	UINT StartIndexLocation;
	INT BaseVertexLocation;
	UINT StartInstanceLocation;
};

class IndirectBuffer
{
public:
	IndirectBuffer(UINT numElements, UINT stride, const void* initData);
	~IndirectBuffer();

	void BindUAV(UINT slot);
	void ClearUAV();
	void SetDataFromBuffer(ComPtr<ID3D11Buffer> _src);

private:

	ComPtr<ID3D11Buffer> _buffer;

	ComPtr<ID3D11UnorderedAccessView> _UAV;

	UINT SlotNum;

public:
	ComPtr<ID3D11Buffer> GetBuffer() { return _buffer; }
};

