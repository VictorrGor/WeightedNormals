#include "Render.h"

RenderSys::RenderSys()
{
	pDevice = nullptr;
	pDeviceContext = nullptr;
	pSwapChain = nullptr;
	pRenderTargetView = nullptr;
	pDepthBuffer = nullptr;
	mCamera = nullptr;
	pModelCB = nullptr;
}

RenderSys::~RenderSys()
{
	Release();
}

HRESULT RenderSys::Initialize(const HWND& hWnd)
{
	HRESULT hRes = S_OK;
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	RECT rc; 
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;


	DXGI_SWAP_CHAIN_DESC ds;
	ZeroMemory(&ds, sizeof(ds));
	ds.BufferCount = 2;
	ds.BufferDesc.Width = width;
	ds.BufferDesc.Height = height;
	ds.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ds.BufferDesc.RefreshRate.Numerator = 60;
	ds.BufferDesc.RefreshRate.Denominator = 1;
	ds.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	ds.OutputWindow = hWnd;
	ds.SampleDesc.Count = 1;
	ds.SampleDesc.Quality = 0;
	ds.Windowed = TRUE;

	hRes = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
		&ds, &pSwapChain, &pDevice, &featureLevel, &pDeviceContext);
	if (FAILED(hRes)) return hRes;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hRes = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hRes))
		return hRes;

	hRes = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hRes))
		return hRes;

	D3D11_TEXTURE2D_DESC depthTextureDesc;
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = width;
	depthTextureDesc.Height = height;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* DepthStencilTexture;
	hRes = pDevice->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);

	if (FAILED(hRes))
		return hRes;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = depthTextureDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hRes = pDevice->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &pDepthBuffer);
	DepthStencilTexture->Release();

	if (FAILED(hRes))
		return hRes;

	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthBuffer);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDeviceContext->RSSetViewports(1, &vp);

	mCamera = new Camera({ 0,0,0 }, { 1,0,0 }, { 0,1,0 }, width, height);

	//Constant Buffers
	D3D11_BUFFER_DESC cb_ds;
	memset(&cb_ds, 0, sizeof(cb_ds));
	cb_ds.ByteWidth = sizeof(CBPerObj);
	cb_ds.Usage = D3D11_USAGE_DEFAULT;;
	cb_ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hRes = pDevice->CreateBuffer(&cb_ds, NULL, &pModelCB);

	return hRes;
}

void RenderSys::Release()
{
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Release();
		delete (*it);
	}
	objects.clear();

	if (pDepthBuffer)
	{
		pDepthBuffer->Release();
		pDepthBuffer = nullptr;
	}
	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
		pRenderTargetView = nullptr;
	}
	if (pSwapChain)
	{
		pSwapChain->Release();
		pSwapChain = nullptr;
	}
	if (pDeviceContext)
	{
		pDeviceContext->Release();
		pDeviceContext = nullptr;
	}
	if (pDevice)
	{
		pDevice->Release();
		pDevice = nullptr;
	}
	if (pModelCB)
	{
		pModelCB->Release();
		pModelCB = nullptr;
	}
}

XMVECTOR calcTriangleNormal(const Vertex& _vt1, const Vertex& _vt2, const Vertex& _vt3)
{
	XMVECTOR u, v, pt;
	pt = XMLoadFloat3(&_vt1.position);
	u  = XMLoadFloat3(&_vt2.position);
	u  = XMVectorSubtract(u, pt);
	v  = XMLoadFloat3(&_vt3.position);
	v  = XMVectorSubtract(v, pt);
	return XMVector3Normalize(XMVector3Cross(u, v));
}

void calcWeightedNormals(Vertex* _vertices, UINT _vtxCount, const UINT* _indicies, UINT _idxCount)
{
	for (UINT iVtx = 0; iVtx < _vtxCount; ++iVtx)
	{
		_vertices[iVtx].normal = { 0,0,0 };
	}
	if (_idxCount % 3 != 0) return;

	UINT* linkCounter = new UINT[_vtxCount];
	memset(linkCounter, 0, sizeof(UINT) * _vtxCount);

	Vertex *vx1, *vx2, *vx3;
	DirectX::XMVECTOR vNormal, vBuf;
	for (UINT iIdx = 0; iIdx < _idxCount; iIdx += 3)
	{
		vx1 = &_vertices[_indicies[iIdx]];
		vx2 = &_vertices[_indicies[iIdx + 1]];
		vx3 = &_vertices[_indicies[iIdx + 2]];
		vNormal = calcTriangleNormal(*vx1, *vx2, *vx3);
		vBuf = XMLoadFloat3(&vx1->normal);
		XMStoreFloat3(&vx1->normal, DirectX::XMVectorAdd(vNormal, vBuf));
		vBuf = XMLoadFloat3(&vx2->normal);
		XMStoreFloat3(&vx2->normal, DirectX::XMVectorAdd(vNormal, vBuf));
		vBuf = XMLoadFloat3(&vx3->normal);
		XMStoreFloat3(&vx3->normal, DirectX::XMVectorAdd(vNormal, vBuf));

		++linkCounter[_indicies[iIdx]];
		++linkCounter[_indicies[iIdx + 1]];
		++linkCounter[_indicies[iIdx + 2]];
	}
	for (UINT iVtx = 0; iVtx < _vtxCount; ++iVtx)
	{
		vNormal = XMLoadFloat3(&_vertices[iVtx].normal);
		vBuf = XMVectorSet(linkCounter[iVtx] , linkCounter[iVtx], linkCounter[iVtx], 1);
		XMStoreFloat3(&_vertices[iVtx].normal, XMVector3Normalize(XMVectorDivide(vNormal, vBuf)));
	}
	delete[] linkCounter;
}

void RenderSys::drawCubeScene()
{
	Vertex cube[8];

	cube[0].position = { 0.5, 0.5, 0.5 };
	cube[0].color = { 1, 0, 0, 1 };
	cube[1].position = { -0.5, 0.5, 0.5 };
	cube[1].color = { 0, 1, 0, 1 };
	cube[2].position = { -0.5, 0.5, -0.5 };
	cube[2].color = { 0, 0, 1, 1 };
	cube[3].position = { 0.5, 0.5, -0.5 };
	cube[3].color = { 1, 0, 0, 1 };
	cube[4].position = { 0.5,  -0.5, 0.5 };
	cube[4].color = { 0,  1, 0, 1 };
	cube[5].position = { -0.5, -0.5, 0.5 };
	cube[5].color = { 0, 0, 1, 1 };
	cube[6].position = { -0.5, -0.5, -0.5 };
	cube[6].color = { 1, 0, 0, 1 };
	cube[7].position = { 0.5,  -0.5, -0.5 };
	cube[7].color = { 0,  1, 0, 1 };

	UINT indexes[] = {0, 2, 1, 0, 3, 2, //up
					0, 7 ,3, 7, 0, 4,   //right
					0, 5, 4, 0, 1 ,5,   // back
					1, 6, 5, 6, 1, 2,   // left
					2, 3, 6, 7, 6, 3,   //face
					7, 5, 6, 5, 7, 4 
	}; 

	ShaderClass* pShader = new ShaderClass();
	pShader->Initialize(pDevice, L"shaders/vertexShader.hlsl", L"shaders/pixelShader.hlsl");
	objects.push_back(createEntity(&cube[0], 8, &indexes[0], ARRAYSIZE(indexes), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, pShader));

	calcWeightedNormals(&cube[0], 8, &indexes[0], ARRAYSIZE(indexes));
	drawNormals(&cube[0], 8);
}

void RenderSys::drawPlaneScene()
{
	Vertex plane[4];

	plane[0].position = { 0.5, 0., 0.5 };
	plane[0].color = { 1, 0, 0, 1 };
	plane[1].position = { -0.5, 0., 0.5 };
	plane[1].color = { 0, 1, 0, 1 };
	plane[2].position = { -0.5, 0., -0.5 };
	plane[2].color = { 0, 0, 1, 1 };
	plane[3].position = { 0.5, 0., -0.5 };
	plane[3].color = { 1, 0, 0, 1 };

	UINT indexes[] = { 0, 2, 1, 0, 3, 2 };

	ShaderClass* pShader = new ShaderClass();
	pShader->Initialize(pDevice, L"shaders/vertexShader.hlsl", L"shaders/pixelShader.hlsl");
	objects.push_back(createEntity(&plane[0], ARRAYSIZE(plane), &indexes[0], ARRAYSIZE(indexes), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, pShader));
	
	calcWeightedNormals(&plane[0], 4, &indexes[0], 6);
	drawNormals(&plane[0], 4);
}

void RenderSys::drawNormals(const Vertex* _vertices, UINT _count)
{
	Vertex* normlas = new Vertex[_count * 2];
	XMVECTOR v1, v2;
	for (UINT iVtx = 0; iVtx < _count; ++iVtx)
	{
		normlas[iVtx * 2].position = _vertices[iVtx].position;
		v1 = XMLoadFloat3(&_vertices[iVtx].position); v2 = XMLoadFloat3(&_vertices[iVtx].normal);
		XMStoreFloat3(&normlas[iVtx * 2 + 1].position, XMVectorAdd(v1, v2));
	}
	ShaderClass* pShader = new ShaderClass();
	pShader->Initialize(pDevice, L"shaders/vertexShader.hlsl", L"shaders/pixelShader.hlsl");
	objects.push_back(createEntity(normlas, _count * 2, nullptr, 0, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, pShader));

}

void flipNormals(Vertex* _vertices, UINT _count)
{
	for (UINT iVtx = 0; iVtx < _count; ++iVtx)
	{
		_vertices[iVtx].normal.x *= -1;
		_vertices[iVtx].normal.y *= -1;
		_vertices[iVtx].normal.z *= -1;
	}
}

void calcNewTriangle(const Vertex& _up, const Vertex& _left, const Vertex& _right,
	Vertex& _downW, Vertex& _leftW, Vertex& _rightW)
{
	XMVECTOR xmLeft, xmRight, xmUp, xmRes;
	xmUp = XMLoadFloat3(&_up.position);
	xmLeft = XMLoadFloat3(&_left.position);
	xmRight = XMLoadFloat3(&_right.position);

	xmRes = (xmLeft + xmRight) / 2;
	XMStoreFloat3(&_downW.position, xmRes);
	xmRes = (xmLeft + xmUp) / 2;
	XMStoreFloat3(&_leftW.position, xmRes);
	xmRes = (xmRight + xmUp) / 2;
	XMStoreFloat3(&_rightW.position, xmRes);
}

UINT getLvlSum(UINT _lvl)
{
	return _lvl * (_lvl + 1) / 2;
}

void tesselateTriangle(Vertex* pVtx, UINT* indices, const UINT& tesselation_lvl, 
			const UINT& vertex_count, const UINT& vertex_lvl, UINT _pointOffset)
{
	UINT upPt = 0, leftPt = vertex_count - vertex_lvl, rightPt = vertex_count - 1;
	UINT upPtLvl = 0, leftPtLvl = vertex_lvl - 1, rightPtLvl = vertex_lvl - 1;
	UINT index_ctr = 0;

	struct triangleDesc {
		UINT upPt;
		UINT leftPt;
		UINT rightPt;
		UINT upPtLvl;
		UINT leftPtLvl;
		UINT rightPtLvl;
	};
	std::queue<triangleDesc> actualTriangles, subTriangles;
	actualTriangles.push({ upPt, leftPt, rightPt, upPtLvl, leftPtLvl, rightPtLvl });
	for (UINT iDivider = 1; iDivider <= tesselation_lvl; ++iDivider)
	{
		while (!actualTriangles.empty())
		{
			triangleDesc& it = actualTriangles.front();
			if (it.upPt < it.leftPt)
			{
				upPt = (it.leftPt + it.rightPt) / 2;
				upPtLvl = it.leftPtLvl;
				leftPtLvl = (it.leftPtLvl + it.upPtLvl) / 2;
				rightPtLvl = leftPtLvl;
				rightPt = it.leftPt - (getLvlSum(upPtLvl - 1) - getLvlSum(leftPtLvl - 1));
				leftPt = rightPt - (it.rightPt - it.leftPt) / 2;

				calcNewTriangle(pVtx[it.upPt], pVtx[it.leftPt], pVtx[it.rightPt], pVtx[upPt], pVtx[leftPt], pVtx[rightPt]);
				subTriangles.push({ it.upPt, leftPt, rightPt, it.upPtLvl, leftPtLvl, rightPtLvl });
				subTriangles.push({ leftPt, it.leftPt, upPt, leftPtLvl, it.leftPtLvl, upPtLvl });
				subTriangles.push({ upPt, leftPt, rightPt, upPtLvl, leftPtLvl, rightPtLvl });
				subTriangles.push({ rightPt, upPt, it.rightPt, rightPtLvl, upPtLvl, it.rightPtLvl });
			}
			else
			{
				upPt = (it.leftPt + it.rightPt) / 2;
				upPtLvl = it.leftPtLvl;
				leftPtLvl = (it.leftPtLvl + it.upPtLvl) / 2;
				rightPtLvl = leftPtLvl;
				leftPt = it.rightPt + (getLvlSum(rightPtLvl - 1) - getLvlSum(upPtLvl - 1));
				rightPt = leftPt + (it.rightPt - it.leftPt) / 2;
				calcNewTriangle(pVtx[it.upPt], pVtx[it.leftPt], pVtx[it.rightPt], pVtx[upPt], pVtx[leftPt], pVtx[rightPt]);

				subTriangles.push({ leftPt, it.leftPt, upPt, leftPtLvl, it.leftPtLvl, upPtLvl });
				subTriangles.push({ upPt, leftPt, rightPt, upPtLvl, leftPtLvl, rightPtLvl });
				subTriangles.push({ rightPt, upPt, it.rightPt, rightPtLvl, upPtLvl, it.rightPtLvl });
				subTriangles.push({ it.upPt, leftPt, rightPt, it.upPtLvl, leftPtLvl, rightPtLvl });
			}
			actualTriangles.pop();
		}
		actualTriangles = std::move(subTriangles);
	}

	for (UINT iLvl = 1; iLvl < vertex_lvl; ++iLvl)
	{
		UINT offset = iLvl * (iLvl - 1) / 2;
		UINT upperBound = iLvl + offset;


		indices[index_ctr] = _pointOffset + offset;
		++index_ctr;
		indices[index_ctr] = _pointOffset + offset + iLvl + 1;
		++index_ctr;
		indices[index_ctr] = _pointOffset + offset + iLvl;
		++index_ctr;
		for (UINT upIndex = offset + 1; upIndex < upperBound; ++upIndex)
		{
			indices[index_ctr] = _pointOffset + upIndex - 1;
			++index_ctr;
			indices[index_ctr] = _pointOffset + upIndex;
			++index_ctr;
			indices[index_ctr] = _pointOffset + upIndex + iLvl;
			++index_ctr;

			indices[index_ctr] = _pointOffset + upIndex;
			++index_ctr;
			indices[index_ctr] = _pointOffset + upIndex + iLvl + 1;
			++index_ctr;
			indices[index_ctr] = _pointOffset + upIndex + iLvl;
			++index_ctr;
		}
	}
}

void RenderSys::genSphere(UINT tesselation_lvl)
{
	//Vertex count per triangle by each tesselation level
	//Tess_lvl = tl E [1, n];
	// vertex_lvl = 2 ^n + 1;
	// vertex_count = 0
	// vertex_count += i for i in range(1, vertex_lvl + 1) => sum = (1 + vertex_lvl) * vertex_lvl / 2

	UINT vertex_lvl = pow(2, tesselation_lvl) + 1;
	UINT vertex_count = (1 + vertex_lvl) * vertex_lvl / 2;
	UINT face_count = 8; 
	UINT full_count = vertex_count * face_count;

	Vertex* pVtx = new Vertex[full_count]; 
	Vertex* pVtxSaved = pVtx;

	UINT triangle_count = pow(4, tesselation_lvl);
	UINT indices_size = triangle_count * 3; 
	UINT full_indices_size = indices_size * face_count;

	UINT* pIndices = new UINT[full_indices_size];
	UINT* pIndicesSaved = pIndices;
	

	memset(&pVtx[0], 0, sizeof(Vertex) * full_count);

	pVtxSaved[0].position = { 0, 1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { 1, 0, -1 };
	pVtxSaved[vertex_count - 1].position = { 1, 0, 1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, 0);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, 1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { 1, 0, 1 };
	pVtxSaved[vertex_count - 1].position = { -1, 0, 1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = {0, 1, 0};
	pVtxSaved[vertex_count - vertex_lvl].position = { -1, 0, 1 };
	pVtxSaved[vertex_count - 1].position = { -1, 0, -1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 2);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, 1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { -1, 0, -1 };
	pVtxSaved[vertex_count - 1].position = { 1, 0, -1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 3);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, -1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { 1, 0, 1 };
	pVtxSaved[vertex_count - 1].position = { 1, 0, -1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 4);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, -1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { -1, 0, 1 };
	pVtxSaved[vertex_count - 1].position = { 1, 0, 1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 5);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, -1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { -1, 0, -1 };
	pVtxSaved[vertex_count - 1].position = { -1, 0, 1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 6);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	pVtxSaved[0].position = { 0, -1, 0 };
	pVtxSaved[vertex_count - vertex_lvl].position = { 1, 0, -1 };
	pVtxSaved[vertex_count - 1].position = { -1, 0, -1 };
	tesselateTriangle(pVtxSaved, pIndicesSaved, tesselation_lvl, vertex_count, vertex_lvl, vertex_count * 7);
	pVtxSaved += vertex_count; pIndicesSaved += indices_size;

	for (UINT it = 0; it < full_count; ++it)
	{
		XMVECTOR v1 = XMLoadFloat3(&pVtx[it].position);
		XMStoreFloat3(&pVtx[it].position, XMVector3Normalize(v1));
	}

	calcWeightedNormals(pVtx, full_count, pIndices, full_indices_size);
	//flipNormals(pVtx, full_count);
	drawNormals(pVtx, full_count);
	ShaderClass* pShader = new ShaderClass();
	pShader->Initialize(pDevice, L"shaders/vertexShader.hlsl", L"shaders/pixelShader.hlsl");
	auto pEnt = createEntity(pVtx, full_count, pIndices, full_indices_size, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, pShader);
	this->objects.push_back(pEnt);
}


void RenderSys::Render()
{
	const static float ClearColor[4] = { 1.0f, 1.f, 1.f, 1.0f };
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, ClearColor);
	pDeviceContext->ClearDepthStencilView(pDepthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	static DWORD startTime = GetTickCount();
	DWORD actualTime = GetTickCount();
	static float R = 3.;
	float t = (actualTime - startTime) / 1000.0f;
	mCamera->setPos({ R * sinf(t) , 2, R * cosf(t) }, { 0, 0, 0 }, { 0, 1, 0 });
	

	for (std::vector<Entity*>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Render(pDeviceContext, *mCamera, nullptr, pModelCB);
	}
	pSwapChain->Present(1, 0);
}

Entity* RenderSys::createEntity(Vertex* _pVx, UINT _vxCount, UINT* _pIndex, UINT _indexCount,
	D3D_PRIMITIVE_TOPOLOGY _topology, ShaderClass* _pShader)
{
	Entity* res = new Entity();
	res->Initialize(createVertexBuffer(_pVx, _vxCount), _vxCount, createIndexBuffer(_pIndex, _indexCount), _indexCount, _topology, _pShader);
	return res;
}

ID3D11Buffer* RenderSys::createVertexBuffer(Vertex* _mem, UINT _ptCount)
{
	ID3D11Buffer* res;
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.ByteWidth = _ptCount * sizeof(Vertex);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA dataStorage;
	memset(&dataStorage, 0, sizeof(dataStorage));
	dataStorage.pSysMem = _mem;
	auto status = pDevice->CreateBuffer(&desc, &dataStorage, &res);
	if (status != S_OK) return nullptr;

	return res;
}

ID3D11Buffer* RenderSys::createIndexBuffer(UINT* _mem, UINT _indexCount)
{
	if (!_mem) return nullptr;
	ID3D11Buffer* res;
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.ByteWidth = _indexCount * sizeof(UINT);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA dataStorage;
	memset(&dataStorage, 0, sizeof(dataStorage));
	dataStorage.pSysMem = _mem;
	auto status = pDevice->CreateBuffer(&desc, &dataStorage, &res);
	if (status != S_OK) return nullptr;

	return res;
}
