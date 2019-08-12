#include "pch.h"
#include "dx11.h"
#include "rayTracer.cpp"

struct CbObject
{
	XMMATRIX m_world;
	XMMATRIX m_view;
	XMMATRIX m_proj;
	XMMATRIX m_worldViewProj;
};

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

class App : public DX11
{
public:
	App(HINSTANCE instance) : DX11(instance)
	{
		m_currentCirclePosX = 0.0f;
		m_currentCirclePosY = 0.0f;
	};

	void onInit() override;
	void onRender() override;
	void onInput() override;
	void onUpdate() override;

public:
	void createLines();
private:
	// Buffers
	ID3D11Buffer *m_vertexBuffer;
	UINT m_numVertices;
	ID3D11Buffer *m_indexBuffer;
	UINT m_numIndices;

	// Constant buffers
	ID3D11Buffer *cbObjectBuffer;

	// Circle
	float m_currentCirclePosX;
	float m_currentCirclePosY;

	// Walls
	std::vector<Line> walls;
};

void App::createLines()
{
	Circle c(Vector3D(5.0f, 5.0f, 0.0f), 2.0f);
	c.placePoints(160);
	
	// TODO: Cube 1
	Line l1(Vector3D(10.0f, -7.0f, 0.0f), Vector3D(10.0f, 7.0f, 0.0f));
	Line l2(Vector3D(10.0f, -7.0f, 0.0f), Vector3D(-10.0f, -7.0f, 0.0f));
	Line l3(Vector3D(10.0f, 7.0f, 0.0f), Vector3D(-10.0f, 7.0f, 0.0f));
	Line l4(Vector3D(-10.0f, -7.0f, 0.0f), Vector3D(-10.0f, 7.0f, 0.0f));

	Line l5(Vector3D(15.0f, 7.0f, 0.0f), Vector3D(15.0f, -7.0f, 0.0f));
	Line l6(Vector3D(15.0f, -7.0f, 0.0f), Vector3D(-15.0f, -10.0f, 0.0f));
	walls.push_back(l1);
	walls.push_back(l2);
	walls.push_back(l3);
	walls.push_back(l4);
	walls.push_back(l5);
	walls.push_back(l6);

	c.intersectPoints(walls);
	
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	UINT numLines = c.circleLines.size();
	UINT numAddLines = walls.size();

	for (int i = 0; i < walls.size(); i++)
	{
		XMFLOAT3 f;
		f.x = walls[i].m_p1.x;
		f.y = walls[i].m_p1.y;
		f.z = 0.0f;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });

		f.x = walls[i].m_p2.x;
		f.y = walls[i].m_p2.y;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });
	}

	for (int i = 0; i < numLines; i++)
	{
		XMFLOAT3 f;
		f.x = c.circleLines[i].m_p1.x;
		f.y = c.circleLines[i].m_p1.y;
		f.z = 0.0f;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });

		f.x = c.circleLines[i].m_p2.x;
		f.y = c.circleLines[i].m_p2.y;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });
	}

	m_numVertices = numLines * 2 + numAddLines * 2;
	m_numIndices = m_numVertices * 2;

	for (int i = 0; i < m_numIndices; i++)
	{
		indices.push_back(i);
	}

	// Vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vertex) * m_numVertices;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	DX::ThrowIfFailed(m_device->CreateBuffer(&bd, &initData, &m_vertexBuffer));

	// Set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	m_deviceContext->UpdateSubresource(m_vertexBuffer, 0, nullptr, &vertices[0], 0, 0);

	// Index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth = sizeof(UINT) * m_numIndices;
	bd.CPUAccessFlags = 0;

	initData.pSysMem = &indices[0];

	DX::ThrowIfFailed(m_device->CreateBuffer(&bd, &initData, &m_indexBuffer));

	// Set index buffer
	m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
}

void App::onInit()
{
	DX11::onInit();

	// Input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(vertexDesc);
	DX::ThrowIfFailed(m_device->CreateInputLayout(vertexDesc, numElements, m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize(), &m_inputLayout));

	// Set input layout
	m_deviceContext->IASetInputLayout(m_inputLayout);

	// Set primitive topology
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Vertex buffer, index buffer
	createLines();

	// Constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(CbObject);
	bd.CPUAccessFlags = 0;
	
	XMVECTOR eye = { 0.0f, 0.0f, -20.0f, 0.0f }; // TODO: make smooth zoom and out
	XMVECTOR at = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	m_view = XMMatrixLookAtLH(eye, at, up);

	m_proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, m_width / (FLOAT)m_height, 0.01f, 1025.0f);

	CbObject cb;
	cb.m_world = XMMatrixTranspose(m_world);
	cb.m_view = XMMatrixTranspose(m_view);
	cb.m_proj = XMMatrixTranspose(m_proj);
	cb.m_worldViewProj = XMMatrixTranspose(m_world * m_view * m_proj);

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &cb;
	DX::ThrowIfFailed(m_device->CreateBuffer(&bd, &initData, &cbObjectBuffer));
}

void App::onInput()
{
	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		m_currentCirclePosX += 1.0f;
	}
	else if (GetKeyState(VK_LEFT) & 0x8000)
	{
		m_currentCirclePosX -= 1.0f;
	}
	else if (GetKeyState(VK_UP) & 0x8000)
	{
		m_currentCirclePosY += 1.0f;
	}
	else if (GetKeyState(VK_DOWN) & 0x8000)
	{
		m_currentCirclePosY -= 1.0f;
	}
}

void App::onUpdate()
{
	Circle c(Vector3D(m_currentCirclePosX, m_currentCirclePosY, 0.0f), 2.0f);
	c.placePoints(160);

	c.intersectPoints(walls);

	UINT numLines = c.circleLines.size();

	std::vector<Vertex> vertices;
	for (int i = 0; i < walls.size(); i++)
	{
		XMFLOAT3 f;
		f.x = walls[i].m_p1.x;
		f.y = walls[i].m_p1.y;
		f.z = 0.0f;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });

		f.x = walls[i].m_p2.x;
		f.y = walls[i].m_p2.y;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });
	}

	for (int i = 0; i < numLines; i++)
	{
		XMFLOAT3 f;
		f.x = c.circleLines[i].m_p1.x;
		f.y = c.circleLines[i].m_p1.y;
		f.z = 0.0f;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });

		f.x = c.circleLines[i].m_p2.x;
		f.y = c.circleLines[i].m_p2.y;
		vertices.push_back(Vertex{ f, (XMFLOAT4)Colors::White });
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(Vertex) * m_numVertices;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];
	
	// Update vertex buffer
	m_deviceContext->UpdateSubresource(m_vertexBuffer, 0, nullptr, &vertices[0], 0, 0);
}

void App::onRender()
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, Colors::Black);
 	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	m_deviceContext->VSSetConstantBuffers(0, 1, &cbObjectBuffer);
	m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	m_deviceContext->DrawIndexed(m_numIndices, 0, 0);

	DX::ThrowIfFailed(m_swapChain->Present(0, 0));
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	App app(hInstance);
	app.onInit();
	app.onRender();
	app.run();

	return 0;
}