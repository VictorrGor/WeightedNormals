#pragma once
#include "Render.h"

class Camera
{
public:
	vec3 eye;
	vec3 up;
	vec3 lookAt;
	mtx mxProjection;
	mtx mxView;
	mtx mxVP;
	Camera(vec3 _eye, vec3 _lookAt, vec3 _up, UINT _width, UINT _height);
	void setPos(vec3 _eye, vec3 _lookAt, vec3 _up);
	const mtx& getViewProjMx();
};