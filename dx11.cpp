#include "pch.h"
#include "dx11.h"

namespace
{
	DX11 *dx11 = 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return dx11->MsgProc(hwnd, msg, wParam, lParam);
}

LRESULT DX11::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DX11::DX11(HINSTANCE instance) :
	m_appInstance(instance),
	m_width(1240),
	m_height(1024)
{
	XMMATRIX i = XMMatrixIdentity();
	m_world = i;
	m_view = i;
	m_proj = i;

	dx11 = this;
}

void DX11::onInit()
{
	initWindow();
	initDX11();
}

typedef std::chrono::steady_clock::time_point TimePoint;
TimePoint(*timeGet)() = std::chrono::high_resolution_clock::now;

double getDuration(TimePoint timeEnd, TimePoint timeStart)
{
	return std::chrono::duration<double>(timeEnd - timeStart).count();
}

void DX11::run()
{
	MSG msg = { 0 };

	unsigned int fps = 0;
	TimePoint lastTime = timeGet();
	double fpsCountTimer = 0;
	double updateTimer = 0;
	float frameTime = 1 / 60.0f;

	while (msg.message != WM_QUIT)
	{
		TimePoint currentTime = timeGet();
		double passedTime = getDuration(currentTime, lastTime);
		lastTime = currentTime;

		updateTimer += passedTime;
		fpsCountTimer += passedTime;

		if (fpsCountTimer >= 1.0)
		{
			std::wostringstream outs;
			outs.precision(6);
			outs << L"Main Window" << L"    "
				<< L"FPS: " << fps << L"    ";
			SetWindowText(m_mainWindow, outs.str().c_str());

			fpsCountTimer = 0;
			fps = 0;
		}

		bool shouldRender = false;
		while (updateTimer >= frameTime)
		{
			onInput();
			onUpdate();
			updateTimer -= frameTime;
			shouldRender = true;
		}

		if (shouldRender)
		{
			onRender();
			fps++;
		}

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}

void DX11::initWindow()
{
	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_appInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"Main Window";

	if (!RegisterClass(&wc))
	{
		__debugbreak();
	}

	RECT rc = { 0, 0, m_width, m_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	m_mainWindow = CreateWindow(
		L"Main Window",
		L"Main Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		0,
		0,
		m_appInstance,
		0);

	if (!m_mainWindow)
	{
		__debugbreak();
	}

	ShowWindow(m_mainWindow, SW_SHOW);
	UpdateWindow(m_mainWindow);
}

void DX11::initDX11()
{
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr;
	// Device
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, D3D11_CREATE_DEVICE_DEBUG, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, 0, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_deviceContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	DX::ThrowIfFailed(hr);

	// Factory
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	DX::ThrowIfFailed(hr);

	// Swap chain
	IDXGIFactory2 *dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = m_device->QueryInterface(IID_PPV_ARGS(&m_device1));
		if (SUCCEEDED(hr))
		{
			(void)m_deviceContext->QueryInterface(IID_PPV_ARGS(&m_deviceContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd; // TODO: MAKE {}
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = m_width;
		sd.Height = m_height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_device, m_mainWindow, &sd, nullptr, nullptr, &m_swapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = m_width;
		sd.BufferDesc.Height = m_height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_mainWindow;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(m_device, &sd, &m_swapChain);
	}

	dxgiFactory->MakeWindowAssociation(m_mainWindow, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();

	DX::ThrowIfFailed(hr);

	// Render target view
	ID3D11Texture2D *backBuffer = nullptr;
	DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	DX::ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView));
	backBuffer->Release();

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_width;
	descDepth.Height = m_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	DX::ThrowIfFailed(m_device->CreateTexture2D(&descDepth, nullptr, &m_depthStencil));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	DX::ThrowIfFailed(m_device->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView));

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)m_width;
	vp.Height = (float)m_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);

	// Vertex shader
	ID3D10Blob *compilationMsgs = nullptr;

	DX::ThrowIfFailed(D3DCompileFromFile(L"mainShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_vertexShaderBlob, &compilationMsgs));
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
	}
	DX::ThrowIfFailed(m_device->CreateVertexShader(m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize(), NULL, &m_vertexShader));

	// Pixel shader
	DX::ThrowIfFailed(D3DCompileFromFile(L"mainShader.hlsl", 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pixelShaderBlob, &compilationMsgs));
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
	}
	DX::ThrowIfFailed(m_device->CreatePixelShader(m_pixelShaderBlob->GetBufferPointer(), m_pixelShaderBlob->GetBufferSize(), NULL, &m_pixelShader));
}

void DX11::onResize()
{

}