#pragma once
#include  "Render.h"
#include <d3dcompiler.h>


class ShaderClass
{
	ID3D11PixelShader* pPxSh;
	ID3D11VertexShader* pVxSh;
	ID3D11InputLayout* pVtxLayout;
public:
	ShaderClass();
	~ShaderClass();
	HRESULT Initialize(ID3D11Device* _pDevice, LPCWSTR _vsName, LPCWSTR _psName);
	void Release();
	void SetShader(ID3D11DeviceContext* _pDeviceContext);
};