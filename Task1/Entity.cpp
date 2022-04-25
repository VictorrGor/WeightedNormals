#include "Entity.h"

Entity::Entity()
{
	pVertexBuffer = nullptr;
	pIndexBuffer = nullptr;
	pShader = nullptr;
}

void Entity::Initialize(ID3D11Buffer* _pVertexBuffer, UINT _uVertexCount, ID3D11Buffer* _pIndexBuffer, UINT _uIndexCount,
	D3D_PRIMITIVE_TOPOLOGY _topology, ShaderClass* _pShader)
{
	vertexCount = _uVertexCount;
	indexCount = _uIndexCount;
	mxModel = DirectX::XMMatrixIdentity();
	pVertexBuffer = _pVertexBuffer;
	pIndexBuffer = _pIndexBuffer;
	topology = _topology;
	pShader = _pShader;
}

void Entity::Release()
{
	if (pVertexBuffer)
	{
		pVertexBuffer->Release();
		pVertexBuffer = nullptr;
	}
	if (pIndexBuffer)
	{
		pIndexBuffer->Release();
		pIndexBuffer = nullptr;
	}
	if (pShader)
	{
		delete pShader;
		pShader = nullptr;
	}
}

Entity::~Entity()
{
	Release();
}

void Entity::Render(ID3D11DeviceContext* _pDeviceContext, Camera& mCam, ID3D11Buffer* _pFrameCB, ID3D11Buffer* _pModelCB)
{
	static const UINT stride = sizeof(Vertex);
	static const UINT offset = 0;
	pShader->SetShader(_pDeviceContext);
	_pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	_pDeviceContext->IASetPrimitiveTopology(topology);
	_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

	static CBPerObj cbModel;
	cbModel.model = DirectX::XMMatrixIdentity();
	cbModel.projection = DirectX::XMMatrixTranspose(mCam.mxProjection);
	cbModel.view = DirectX::XMMatrixTranspose(mCam.mxView);
	_pDeviceContext->UpdateSubresource(_pModelCB, 0, NULL, &cbModel, 0, 0);
	_pDeviceContext->VSSetConstantBuffers(0, 1, &_pModelCB);


	if (pIndexBuffer) _pDeviceContext->DrawIndexed(indexCount, 0, 0);
	else _pDeviceContext->Draw(vertexCount, 0);
}

void Entity::SetTopology(D3D_PRIMITIVE_TOPOLOGY _topology)
{
	topology = _topology;
}
