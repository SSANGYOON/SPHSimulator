#pragma once

#include "ConstantBuffer.h"

class Texture;
class IFW1Factory;
class IFW1FontWrapper;

struct WindowInfo
{
	HWND hwnd;
	unsigned int width;
	unsigned int height;
	bool windowed;
	bool VSync;
};

class Graphics
{
public:
	Graphics() = default;
	~Graphics() = default;

	HRESULT Init(const WindowInfo& info);
	void SetVSync(bool enabled) { _window.VSync = enabled; }

	ID3D11Device* GetDevice() { return _device.Get(); }
	ID3D11DeviceContext* GetContext() { return _context.Get(); }
	IDXGISwapChain* GetSwapChain() { return _swapChain.Get(); }

	void SetWindow(WindowInfo info);

	void SetViewport(UINT left, UINT bottom, UINT right, UINT top);
	WindowInfo GetWindow() { return _window; }
	shared_ptr<ConstantBuffer> GetConstantBuffer(Constantbuffer_Type type) { return _constantBuffers[static_cast<UINT8>(type)]; }

	void BindSwapChain();
	void ClearSwapChain();

	void Quit();

public:
	IFW1FontWrapper* mFontWrapper;
	ID3D11DepthStencilView* GetCommonDepth() { return _dsv.Get(); }
private:
	WindowInfo _window;
	D3D11_VIEWPORT _viewPort;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11Debug> _debug;
	ComPtr<ID3D11DeviceContext> _context;
	ComPtr<IDXGISwapChain> _swapChain;

	ComPtr<ID3D11Texture2D> _renderTarget;
	ComPtr<ID3D11RenderTargetView> _rtv;

	ComPtr<ID3D11Texture2D> _depthStencilText;
	ComPtr<ID3D11DepthStencilView> _dsv;
	ComPtr<ID3D11UnorderedAccessView> _depthUAV;
	IFW1Factory* mFW1Factory;

	array<shared_ptr<ConstantBuffer>, (size_t)Constantbuffer_Type::END> _constantBuffers;
};