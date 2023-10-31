#include "pch.h"
#include "Texture.h"
#include "Graphics.h"




Texture::Texture()
	:Resource(RESOURCE_TYPE::TEXTURE), mDesc{}
{
}

Texture::~Texture()
{
}

void Texture::Clear(UINT startSlot)
{
	ID3D11ShaderResourceView* srv = nullptr;

	CONTEXT->VSSetShaderResources(startSlot, 1, &srv);
	CONTEXT->DSSetShaderResources(startSlot, 1, &srv);
	CONTEXT->GSSetShaderResources(startSlot, 1, &srv);
	CONTEXT->HSSetShaderResources(startSlot, 1, &srv);
	CONTEXT->CSSetShaderResources(startSlot, 1, &srv);
	CONTEXT->PSSetShaderResources(startSlot, 1, &srv);
}

HRESULT Texture::Load(const std::wstring& path, bool stockObject)
{
	std::wstring fullPath;
	std::filesystem::path parentPath = std::filesystem::current_path().parent_path();
	fullPath = parentPath.wstring() + L"\\Resources\\" + path;

	wchar_t szExtension[256] = {};
	_wsplitpath_s(path.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, szExtension, 256);

	std::wstring extension(szExtension);

	if (extension == L".dds" || extension == L".DDS")
	{
		TexMetadata meta;
		if (FAILED(LoadFromDDSFile(fullPath.c_str(), DDS_FLAGS::DDS_FLAGS_NONE, &meta, _image)))
			return S_FALSE;
		else
		{
			int a = 0;
		}
	}
	else if (extension == L".tga" || extension == L".TGA")
	{
		if (FAILED(LoadFromTGAFile(fullPath.c_str(), nullptr, _image)))
			return S_FALSE;
	}
	else // WIC (png, jpg, jpeg, bmp )
	{
		if (FAILED(LoadFromWICFile(fullPath.c_str(), WIC_FLAGS::WIC_FLAGS_NONE, nullptr, _image)))
			return S_FALSE;
	}

	CreateShaderResourceView
	(
		DEVICE,
		_image.GetImages(),
		_image.GetImageCount(),
		_image.GetMetadata(),
		_SRV.GetAddressOf()
	);
	std::filesystem::path pathObj = std::filesystem::path(fullPath);

	_SRV->GetResource((ID3D11Resource**)_texture.GetAddressOf());
	D3D11_TEXTURE2D_DESC desc;
	_texture->GetDesc(&desc);
	_size = { float(desc.Width), float(desc.Height) };
	return S_OK;
}

bool Texture::Create(UINT width, UINT height, DXGI_FORMAT format, UINT bindFlag, UINT cpuAccess)
{
	//Depth stencil texture
	_size.x = width;
	_size.y = height;

	mDesc.CPUAccessFlags = cpuAccess;
	if (cpuAccess == D3D11_CPU_ACCESS_READ)
		mDesc.Usage = D3D11_USAGE_STAGING;
	else
		mDesc.Usage = D3D11_USAGE_DEFAULT;
	mDesc.Format = format;
	mDesc.Width = width;
	mDesc.Height = height;
	mDesc.ArraySize = 1;
	mDesc.BindFlags = bindFlag;
	mDesc.SampleDesc.Count = 1;
	mDesc.SampleDesc.Quality = 0;

	mDesc.MipLevels = 1;
	mDesc.MiscFlags = 0;

	if (FAILED(DEVICE->CreateTexture2D(&mDesc, nullptr, _texture.GetAddressOf())))
		return false;

	if (bindFlag & D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		if (FAILED(DEVICE->CreateRenderTargetView(_texture.Get(), &renderTargetViewDesc, _RTV.GetAddressOf())))
			return false;
	}

	if (bindFlag & D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL)
	{
		if (FAILED(DEVICE->CreateDepthStencilView(_texture.Get(), nullptr, _DSV.GetAddressOf())))
			return false;
	}

	if (bindFlag & D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = format;
		SRVDesc.Texture2D.MipLevels = 1;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;

		if (FAILED(DEVICE->CreateShaderResourceView(_texture.Get(), &SRVDesc, _SRV.GetAddressOf())))
			return false;
	}

	if (bindFlag & D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.Format = format;
		UAVDesc.Texture2D.MipSlice = 0;
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION::D3D11_UAV_DIMENSION_TEXTURE2D;

		if (FAILED(DEVICE->CreateUnorderedAccessView(_texture.Get(), &UAVDesc, _UAV.GetAddressOf())))
			return false;
	}

	return true;
}

void Texture::BindSRV(ShaderStage stage, UINT slot)
{
	switch (stage)
	{
	case ShaderStage::VS:
		CONTEXT->VSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	case ShaderStage::HS:
		CONTEXT->HSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	case ShaderStage::DS:
		CONTEXT->DSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	case ShaderStage::GS:
		CONTEXT->GSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	case ShaderStage::PS:
		CONTEXT->PSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	case ShaderStage::CS:
		CONTEXT->CSSetShaderResources(slot, 1, _SRV.GetAddressOf());
		break;
	default:
		break;
	}
}

void Texture::ClearSRV(ShaderStage stage, UINT slot)
{
	ID3D11ShaderResourceView* srv = nullptr;

	switch (stage)
	{
	case ShaderStage::VS:
		CONTEXT->VSSetShaderResources(slot, 1, &srv);
		break;
	case ShaderStage::HS:
		CONTEXT->HSSetShaderResources(slot, 1, &srv);
		break;
	case ShaderStage::DS:
		CONTEXT->DSSetShaderResources(slot, 1, &srv);
		break;
	case ShaderStage::GS:
		CONTEXT->GSSetShaderResources(slot, 1, &srv);
		break;
	case ShaderStage::PS:
		CONTEXT->PSSetShaderResources(slot, 1, &srv);
		break;
	case ShaderStage::CS:
		CONTEXT->CSSetShaderResources(slot, 1, &srv);
		break;
	default:
		break;
	}
}

void Texture::BindUAV(UINT slot)
{
	UINT i = -1;
	CONTEXT->CSSetUnorderedAccessViews(slot, 1, _UAV.GetAddressOf(), &i);
}

void Texture::ClearUAV(UINT slot)
{
	ID3D11UnorderedAccessView* uav = nullptr;
	UINT i = -1;
	CONTEXT->CSSetUnorderedAccessViews(slot, 1, &uav, &i);
}

ID3D11RenderTargetView* Texture::GetRTV()
{
	return _RTV.Get();
}

ID3D11RenderTargetView** Texture::GetRTVRef()
{
	return _RTV.GetAddressOf();
}