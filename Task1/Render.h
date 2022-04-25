#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

typedef DirectX::XMFLOAT3 vec3;
typedef DirectX::XMFLOAT4 vec4;
typedef DirectX::XMMATRIX mtx;

#include "Camera.h"
#include "Entity.h"
#include <queue>


class Entity;
class ShaderClass;
class Camera;


struct Vertex
{
	vec3 position;
	vec3 normal;
	vec4 color;
};

struct CBPerObj
{
	mtx model;
	mtx view;
	mtx projection;
};

using namespace DirectX;

/// 
/// @todo Window resize function. Recalculate projectionMx
/// 
class RenderSys
{
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	IDXGISwapChain* pSwapChain;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11DepthStencilView* pDepthBuffer;
	ID3D11Buffer* pModelCB;

	Camera* mCamera;
	std::vector<Entity*> objects;
public:
	RenderSys();
	~RenderSys();
	HRESULT Initialize(const HWND& hWnd);
	void Release();
	void drawCubeScene();
	void drawPlaneScene();
	void Render();
	void drawNormals(const Vertex* _vertices, UINT _count);
	void genSphere(UINT tesselation_lvl);

	Entity* createEntity(Vertex* _pVx, UINT _vxCount, UINT* _pIndex, UINT _indexCount, D3D_PRIMITIVE_TOPOLOGY _topology, ShaderClass* _pShader);
	ID3D11Buffer* createVertexBuffer(Vertex* _mem, UINT _ptCount);
	ID3D11Buffer* createIndexBuffer(UINT* _mem, UINT _indexCount);
};