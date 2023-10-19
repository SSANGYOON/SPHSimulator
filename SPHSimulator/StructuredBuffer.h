#pragma once

class StructuredBuffer
{

public:
	StructuredBuffer();
	~StructuredBuffer();

	bool Create(UINT size, UINT stride, void* data, bool asUAV = false, bool _readable = false);

	void SetData(void* data, UINT bufferCount);
	void GetData(void* data);

	UINT GetStride() { return _stride; }

	void BindSRV(UINT slot);
	void BindUAV(UINT slot);

	void Clear();

private:
	void setDiscription();
	bool createBuffer(void* data);
	bool createView();

	bool createReadBuffer();

private:
	ComPtr<ID3D11Buffer> _buffer;
	ComPtr<ID3D11Buffer> _readBuffer;

	ComPtr<ID3D11ShaderResourceView> _SRV;
	ComPtr<ID3D11UnorderedAccessView> _UAV;

	D3D11_BUFFER_DESC desc;

	UINT _size;
	UINT _stride;

	UINT _SRVSlot = -1;
	UINT _UAVSlot = -1;

	bool _asUAV;
	bool _readable;
};


