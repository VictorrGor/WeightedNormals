#include "Camera.h"
#include <DirectXMath.h>


Camera::Camera(vec3 _eye, vec3 _lookAt, vec3 _up, UINT _width, UINT _height) : eye(_eye), lookAt(_lookAt), up(_up)
{
	mxProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, (FLOAT)_width / _height, 0.1, 100);

	DirectX::XMVECTOR vEye, vLookAt, vUp;
	vEye = XMLoadFloat3(&eye);
	vLookAt = XMLoadFloat3(&lookAt);
	vUp = XMLoadFloat3(&up);
	mxView = DirectX::XMMatrixLookToLH(vEye, vLookAt, vUp);

	mxVP = mxProjection * mxView;
};
void Camera::setPos(vec3 _eye, vec3 _lookAt, vec3 _up)
{
	eye = _eye;
	lookAt = _lookAt;
	up = _up;

	DirectX::XMVECTOR vEye, vLookAt, vUp;
	vEye = XMLoadFloat3(&eye);
	vLookAt = XMLoadFloat3(&lookAt);
	vUp = XMLoadFloat3(&up);
	mxView = DirectX::XMMatrixLookAtLH(vEye, vLookAt, vUp);

	mxVP = mxProjection * mxView;
};
const mtx& Camera::getViewProjMx() { return mxView * mxProjection; };