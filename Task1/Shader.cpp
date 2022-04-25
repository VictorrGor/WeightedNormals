#include "Shader.h"

ShaderClass::ShaderClass()
{
	pPxSh = nullptr;
	pVxSh = nullptr;
	pVtxLayout = nullptr;
}

ShaderClass::~ShaderClass()
{
	Release();
}

HRESULT ShaderClass::Initialize(ID3D11Device* _pDevice, LPCWSTR _vsName, LPCWSTR _psName)
{
	HRESULT hRes = S_OK;
	ID3DBlob* pShaderBuffer = nullptr, * pErrorBuffer = nullptr;

	//VS
	hRes = D3DCompileFromFile(_vsName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", NULL, NULL, &pShaderBuffer, &pErrorBuffer);
	if (pErrorBuffer)
	{
		pErrorBuffer->Release();
		pErrorBuffer = nullptr;
	}
	if (FAILED(hRes))
	{
		if (pShaderBuffer) pShaderBuffer->Release();
		return hRes;
	}
	hRes = _pDevice->CreateVertexShader(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), NULL, &pVxSh);
	if (FAILED(hRes))
	{
		if (pShaderBuffer) pShaderBuffer->Release();
		return hRes;
	}
	//InputLayout
	D3D11_INPUT_ELEMENT_DESC elemDesc[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} };
	UINT numElements = ARRAYSIZE(elemDesc);
	hRes = _pDevice->CreateInputLayout(elemDesc, numElements, pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), &pVtxLayout);
	if (pShaderBuffer)
	{
		pShaderBuffer->Release();
		pShaderBuffer = nullptr;
	}
	if (FAILED(hRes))
	{
		return hRes;
	}
	//PS
	hRes = D3DCompileFromFile(_psName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", NULL, NULL, &pShaderBuffer, &pErrorBuffer);
	if (pErrorBuffer)
	{
		pErrorBuffer->Release();
		pErrorBuffer = nullptr;
	}
	if (FAILED(hRes))
	{
		return hRes;
	}
	hRes = _pDevice->CreatePixelShader(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), NULL, &pPxSh);
	if (pShaderBuffer)
	{
		pShaderBuffer->Release();
		pShaderBuffer = nullptr;
	}
	if (FAILED(hRes))
	{
		return hRes;
	}
	return hRes;
}
void ShaderClass::Release()
{
	if (pVtxLayout)
	{
		pVtxLayout->Release();
		pVtxLayout = nullptr;
	}
	if (pVxSh)
	{
		pVxSh->Release();
		pVxSh = nullptr;
	}
	if (pPxSh)
	{
		pPxSh->Release();
		pPxSh = nullptr;
	}
}

void ShaderClass::SetShader(ID3D11DeviceContext* _pDeviceContext)
{
	_pDeviceContext->VSSetShader(pVxSh, NULL, 0);
	_pDeviceContext->PSSetShader(pPxSh, NULL, 0);
	_pDeviceContext->IASetInputLayout(pVtxLayout);
}
