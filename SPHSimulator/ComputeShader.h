#pragma once
#include "Resource.h"

class ComputeShader : public Resource
{
public:
	ComputeShader();
	~ComputeShader();

	virtual HRESULT Load(const std::wstring& path, bool stockObject) override;

	bool Create(const std::wstring& file);
	void Dispatch();
	FORCEINLINE void SetThreadGroups(UINT X, UINT Y, UINT Z) { _TheadGroupCountX = X; _TheadGroupCountY = Y; _TheadGroupCountZ = Z; }

protected:
	ComPtr<ID3DBlob> _CSBlob;
	ComPtr<ID3D11ComputeShader> _CS;

	UINT _TheadGroupCountX;
	UINT _TheadGroupCountY;
	UINT _TheadGroupCountZ;
};