#pragma once

class InstancingBuffer
{
public:
	InstancingBuffer();
	~InstancingBuffer();

	void Init(UINT maxCount = 10);

	void Clear();
	void AddData(Vector3& params);
	void PushData(const vector<Vector3>& data);
	void PushData();
	void SetDataFromBuffer(ComPtr<ID3D11Buffer> _src);

	UINT						_count = 0;

public:
	UINT						GetCount() { return static_cast<UINT>(_data.size()); }
	ID3D11Buffer* GetBuffer() { return _buffer.Get(); }

	void	SetID(UINT64 instanceId) { _instanceId = instanceId; }
	UINT64	GetID() { return _instanceId; }

private:
	UINT64						_instanceId = 0;
	ComPtr<ID3D11Buffer>		_buffer;

	UINT						_maxCount = 0;
	
	vector<Vector3>	_data;
};