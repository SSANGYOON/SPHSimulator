#pragma once
#include "Resource.h"
#include "Enums.h"

#include "DirectXTex.h"

#pragma comment(lib, "DirectXTex.lib") 

class CubeTex : public Resource
{
public:
	CubeTex();
	virtual ~CubeTex();

	static void Clear(UINT startSlot);

	virtual HRESULT Load(const std::wstring& path, bool stockObject = true) override;

private:
	DirectX::ScratchImage _image;
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _SRV;
	D3D11_TEXTURE2D_DESC mDesc;
	Vector2 _size;
};