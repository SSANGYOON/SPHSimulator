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
	void SetShaderGroup(UINT X, UINT Y, UINT Z) { mThreadGroupCountX = X; mThreadGroupCountY = Y; mThreadGroupCountZ = Z; }

protected:
	ComPtr<ID3DBlob> _CSBlob;
	ComPtr<ID3D11ComputeShader> _CS;

	UINT mThreadGroupCountX;
	UINT mThreadGroupCountY;
	UINT mThreadGroupCountZ;
};