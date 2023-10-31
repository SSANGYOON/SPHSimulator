#include "pch.h"
#include "PipelineStates.h"
#include "Graphics.h"


ComPtr<ID3D11SamplerState> samplerStates[(UINT)SamplerType::End] = {};
ComPtr<ID3D11RasterizerState> rasterizerStates[(UINT)RSType::End] = {};
ComPtr<ID3D11DepthStencilState> depthstencilStates[(UINT)DSType::End] = {};
ComPtr<ID3D11BlendState> blendStates[(UINT)BSType::End] = {};

void SetUpState()
{
#pragma region sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	//D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
	//D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;


	DEVICE->CreateSamplerState
	(
		&samplerDesc
		, samplerStates[(UINT)SamplerType::Point].GetAddressOf()
	);

	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	DEVICE->CreateSamplerState
	(
		&samplerDesc
		, samplerStates[(UINT)SamplerType::Linear].GetAddressOf()
	);

	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
	DEVICE->CreateSamplerState
	(
		&samplerDesc
		, samplerStates[(UINT)SamplerType::Anisotropic].GetAddressOf()
	);

	CONTEXT->PSSetSamplers((UINT)SamplerType::Point
		, 1, samplerStates[(UINT)SamplerType::Point].GetAddressOf());
	CONTEXT->CSSetSamplers((UINT)SamplerType::Point
		, 1, samplerStates[(UINT)SamplerType::Point].GetAddressOf());

	CONTEXT->PSSetSamplers((UINT)SamplerType::Linear
		, 1, samplerStates[(UINT)SamplerType::Linear].GetAddressOf());

	CONTEXT->CSSetSamplers((UINT)SamplerType::Linear
		, 1, samplerStates[(UINT)SamplerType::Linear].GetAddressOf());

	CONTEXT->PSSetSamplers((UINT)SamplerType::Anisotropic
		, 1, samplerStates[(UINT)SamplerType::Anisotropic].GetAddressOf());

	CONTEXT->CSSetSamplers((UINT)SamplerType::Anisotropic
		, 1, samplerStates[(UINT)SamplerType::Anisotropic].GetAddressOf());

#pragma endregion
#pragma region Rasterizer state
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rsDesc.DepthClipEnable = true;

	DEVICE->CreateRasterizerState(&rsDesc
		, rasterizerStates[(UINT)RSType::SolidBack].GetAddressOf());

	rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;

	DEVICE->CreateRasterizerState(&rsDesc
		, rasterizerStates[(UINT)RSType::SolidFront].GetAddressOf());

	rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rsDesc.DepthClipEnable = true;

	DEVICE->CreateRasterizerState(&rsDesc
		, rasterizerStates[(UINT)RSType::SolidNone].GetAddressOf());

	rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rsDesc.DepthClipEnable = true;

	DEVICE->CreateRasterizerState(&rsDesc
		, rasterizerStates[(UINT)RSType::WireframeNone].GetAddressOf());


#pragma endregion
#pragma region Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;

	DEVICE->CreateDepthStencilState(&dsDesc
		, depthstencilStates[(UINT)DSType::Less].GetAddressOf());

	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;

	DEVICE->CreateDepthStencilState(&dsDesc
		, depthstencilStates[(UINT)DSType::Greater].GetAddressOf());

	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.StencilEnable = false;

	DEVICE->CreateDepthStencilState(&dsDesc
		, depthstencilStates[(UINT)DSType::NoWrite].GetAddressOf());

	dsDesc.DepthEnable = false;
	dsDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.StencilEnable = false;

	DEVICE->CreateDepthStencilState(&dsDesc
		, depthstencilStates[(UINT)DSType::None].GetAddressOf());

#pragma endregion
#pragma region Blend State

	blendStates[(UINT)BSType::Default] = nullptr;

	D3D11_BLEND_DESC bsDesc = {};
	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = true;

	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_MAX;
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DEVICE->CreateBlendState(&bsDesc, blendStates[(UINT)BSType::AlphaBlend].GetAddressOf());

	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = false;

	bsDesc.RenderTarget[0].BlendEnable = true;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DEVICE->CreateBlendState(&bsDesc, blendStates[(UINT)BSType::OneOne].GetAddressOf());

	bsDesc = {};

	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = false;

	bsDesc.RenderTarget[0].BlendEnable = TRUE;
	bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bsDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	DEVICE->CreateBlendState(&bsDesc, blendStates[(UINT)BSType::Additive].GetAddressOf());
#pragma endregion
}