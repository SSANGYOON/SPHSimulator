#pragma once

enum class SamplerType
{
	Point,
	Linear,
	Anisotropic,
	End,
};

enum class RSType
{
	SolidBack,
	SolidFront,
	SolidNone,
	WireframeNone,
	End,
};

enum class DSType
{
	Less,
	Greater,
	NoWrite,
	None,
	End,
};

enum class BSType
{
	Default,
	AlphaBlend,
	OneOne,
	End,
};

void SetUpState();
extern ComPtr<ID3D11SamplerState> samplerStates[];
extern ComPtr<ID3D11RasterizerState> rasterizerStates[];
extern ComPtr<ID3D11DepthStencilState> depthstencilStates[];
extern ComPtr<ID3D11BlendState> blendStates[];