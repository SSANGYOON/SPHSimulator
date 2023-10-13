#include "pch.h"
#include "ComputeShader.h"
#include "Graphics.h"

ComputeShader::ComputeShader()
	: Resource(RESOURCE_TYPE::COMPUTE_SHADER)
	, _CSBlob(nullptr)
	, _CS(nullptr)
	, mThreadGroupCountX(1)
	, mThreadGroupCountY(1)
	, mThreadGroupCountZ(1)
{
}

ComputeShader::~ComputeShader()
{
}

HRESULT ComputeShader::Load(const std::wstring& path, bool stockObject)
{
	return E_NOTIMPL;
}

bool ComputeShader::Create(const std::wstring& file)
{
	ComPtr<ID3DBlob> mErrorBlob = nullptr;

	std::filesystem::path path = std::filesystem::current_path().parent_path();
	path += "\\SHADER_SOURCE\\";

	std::wstring shaderPath(path.c_str());
	shaderPath += file;

	D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
		, "CS_MAIN", "cs_5_0", 0, 0
		, _CSBlob.GetAddressOf()
		, mErrorBlob.GetAddressOf());

	if (mErrorBlob)
	{
		OutputDebugStringA((char*)mErrorBlob->GetBufferPointer());
		//mErrorBlob->Release();
	}

	DEVICE->CreateComputeShader(_CSBlob->GetBufferPointer()
		, _CSBlob->GetBufferSize()
		, nullptr
		, _CS.GetAddressOf());

	return true;
}

void ComputeShader::Dispatch()
{
	CONTEXT->CSSetShader(_CS.Get(), nullptr, 0);
	CONTEXT->Dispatch(mThreadGroupCountX, mThreadGroupCountY, mThreadGroupCountZ);
}

