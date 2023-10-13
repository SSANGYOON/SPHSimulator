#include "pch.h"
#include "Graphics.h"
#include "PipelineStates.h"
//#include "Resources.h"

HRESULT Graphics::Init(const WindowInfo& info)
{
	_window = info;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = info.width;
	sd.BufferDesc.Height = info.height;
	sd.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 144;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = info.hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = info.windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, _swapChain.GetAddressOf(), _device.GetAddressOf(), &featureLevel, _context.GetAddressOf());
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, _swapChain.GetAddressOf(), _device.GetAddressOf(), &featureLevel, _context.GetAddressOf());
	if (res != S_OK)
		return res;

	GET_SINGLE(Input)->Init(info.hwnd);
	GET_SINGLE(Timer)->Init();
	//GET_SINGLE(Resources)->CreateDefaultResource();

	_swapChain->GetBuffer(0, IID_PPV_ARGS(_renderTarget.GetAddressOf()));
	_device->CreateRenderTargetView(_renderTarget.Get(), nullptr, _rtv.GetAddressOf());

	D3D11_TEXTURE2D_DESC depthTextureDesc{};

	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.Width = _window.width;
	depthTextureDesc.Height = _window.height;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.MiscFlags = 0;

	if (FAILED(res = DEVICE->CreateTexture2D(&depthTextureDesc, nullptr, _depthStencilText.GetAddressOf())))
		return res;

	if (FAILED(res = DEVICE->CreateDepthStencilView(_depthStencilText.Get(), nullptr, _dsv.GetAddressOf())))
		return res;


	RECT rt = { 0, 0, (LONG)info.width , (LONG)info.height };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);
	SetWindowPos(info.hwnd, nullptr, 0, 0, rt.right - rt.left, rt.bottom - rt.top, 0);
	ShowWindow(info.hwnd, true);
	UpdateWindow(info.hwnd);

	_viewPort = { 0.0f, 0.0f, (float)_window.width, (float)_window.height, 0.0f, 1.0f };

	SetUpState();

	_constantBuffers[(UINT8)Constantbuffer_Type::TRANSFORM] = make_shared<ConstantBuffer>();
	_constantBuffers[(UINT8)Constantbuffer_Type::TRANSFORM]->Init(Constantbuffer_Type::TRANSFORM, sizeof(TransformCB));

	_constantBuffers[(UINT8)Constantbuffer_Type::LIGHT] = make_shared<ConstantBuffer>();
	_constantBuffers[(UINT8)Constantbuffer_Type::LIGHT]->Init(Constantbuffer_Type::LIGHT, sizeof(LightCB));

	return S_OK;
}

void Graphics::SetWindow(WindowInfo info)
{
	_window = info;
	_swapChain->ResizeBuffers(0, info.width, info.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
}

void Graphics::SetViewport(UINT left, UINT right, UINT width, UINT height)
{
	_viewPort.TopLeftX = (float)left;
	_viewPort.TopLeftY = (float)right;
	_viewPort.Width = (float)width;
	_viewPort.Height = (float)height;

	CONTEXT->RSSetViewports(1, &_viewPort);
}

void Graphics::BindSwapChain()
{
	CONTEXT->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsv.Get());
}

void Graphics::ClearSwapChain()
{
	float clearColor[4] = { 0.1f,0.1f,0.1f,1.f };
	CONTEXT->ClearRenderTargetView(_rtv.Get(), clearColor);

	CONTEXT->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}