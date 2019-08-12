#include "pch.h"

class DX11
{
public:
	DX11(HINSTANCE instance);
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void onInit();
	void onResize();
	virtual void onInput() = 0;
	virtual void onUpdate() = 0;
	virtual void onRender() = 0;

	void run();
protected:
	HINSTANCE m_appInstance;
	HWND m_mainWindow;
	D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device *m_device;
	ID3D11Device1 *m_device1;
	ID3D11DeviceContext *m_deviceContext;
	ID3D11DeviceContext1 *m_deviceContext1;
	IDXGISwapChain *m_swapChain;
	IDXGISwapChain1 *m_swapChain1;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11Texture2D *m_depthStencil;
	ID3D11DepthStencilView *m_depthStencilView;
	ID3D11VertexShader *m_vertexShader;
	ID3D10Blob *m_vertexShaderBlob;
	ID3D11PixelShader *m_pixelShader;
	ID3D10Blob *m_pixelShaderBlob;
	ID3D11InputLayout *m_inputLayout;

	XMMATRIX m_world;
	XMMATRIX m_view;
	XMMATRIX m_proj;

	int m_width;
	int m_height;
private:
	void initWindow();
	void initDX11();
};