#include "pch.h"
#include "Shader.h"
#include "Graphics.h"

Shader::Shader()
	:Resource(RESOURCE_TYPE::GRAPHIC_SHADER)
	, _info{}
{
}

Shader::~Shader()
{
}

HRESULT Shader::Load(const wstring& path, bool stockObject)
{
	return E_NOTIMPL;
}

void Shader::CreateShader(const ShaderInfo& info, const ShaderEntry& entry, const wstring& file, bool noInstanceBuffer)
{
	std::filesystem::path path = std::filesystem::current_path().parent_path();
	path += "\\SHADER_SOURCE\\";

	std::wstring shaderPath(path.c_str());
	shaderPath += file;
	_info = info;
	if (entry.VS)
	{
		D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
			, "VS_MAIN", "vs_5_0", 0, 0
			, _VSBlob.GetAddressOf()
			, _ErrorBlob.GetAddressOf());

		if (_ErrorBlob)
		{
			OutputDebugStringA((char*)_ErrorBlob->GetBufferPointer());
			//_ErrorBlob->Release();
		}

		DEVICE->CreateVertexShader(_VSBlob->GetBufferPointer()
			, _VSBlob->GetBufferSize()
			, nullptr
			, _VS.GetAddressOf());

		if (noInstanceBuffer)
		{
			D3D11_INPUT_ELEMENT_DESC arrLayoutDesc[] = {

			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			
			};

			DEVICE->CreateInputLayout(arrLayoutDesc, 4
				, _VSBlob->GetBufferPointer()
				, _VSBlob->GetBufferSize()
				, _inputLayout.GetAddressOf());
		}

		else {

			//TODO


			D3D11_INPUT_ELEMENT_DESC arrLayoutDesc[] = {

			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};

			DEVICE->CreateInputLayout(arrLayoutDesc, 14
				, _VSBlob->GetBufferPointer()
				, _VSBlob->GetBufferSize()
				, _inputLayout.GetAddressOf());
		}
	}

	if (entry.PS)
	{
		D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
			, "PS_MAIN", "ps_5_0", 0, 0
			, _PSBlob.GetAddressOf()
			, _ErrorBlob.GetAddressOf());

		if (_ErrorBlob)
		{
			OutputDebugStringA((char*)_ErrorBlob->GetBufferPointer());
			//_ErrorBlob->Release();
		}

		DEVICE->CreatePixelShader(_PSBlob->GetBufferPointer()
			, _PSBlob->GetBufferSize()
			, nullptr
			, _PS.GetAddressOf());
	}

	if (entry.GS)
	{
		D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
			, "GS_MAIN", "gs_5_0", 0, 0
			, _GSBlob.GetAddressOf()
			, _ErrorBlob.GetAddressOf());

		if (_ErrorBlob)
		{
			OutputDebugStringA((char*)_ErrorBlob->GetBufferPointer());
			//	mErrorBlob->Release();
		}

		DEVICE->CreateGeometryShader(_GSBlob->GetBufferPointer()
			, _GSBlob->GetBufferSize()
			, nullptr
			, _GS.GetAddressOf());
	}
}

void Shader::BindShader()
{
	CONTEXT->IASetPrimitiveTopology(_info.topology);
	CONTEXT->IASetInputLayout(_inputLayout.Get());

	CONTEXT->VSSetShader(_VS.Get(), nullptr, 0);
	CONTEXT->PSSetShader(_PS.Get(), nullptr, 0);
	CONTEXT->GSSetShader(_GS.Get(), nullptr, 0);

	ComPtr<ID3D11RasterizerState> rs = rasterizerStates[(UINT)_info.rst];
	ComPtr<ID3D11DepthStencilState> ds = depthstencilStates[(UINT)_info.dst];
	ComPtr<ID3D11BlendState> bs = blendStates[(UINT)_info.bst];

	CONTEXT->RSSetState(rs.Get());
	CONTEXT->OMSetDepthStencilState(ds.Get(), 0);
	CONTEXT->OMSetBlendState(bs.Get(), nullptr, 0xffffffff);
}
