#pragma once
#include <DirectXMath.h>
#include <string>

#define POSITIVE 1
#define NEGATIVE -1

enum Axis
{
	X, 
	Y,
	Z
};


class Camera
{
public:
	Camera();

	void moveCamera(Axis axis, int sign);

	DirectX::XMFLOAT3 getPosition();
	DirectX::XMMATRIX getTransform();

public:
	float translationSpeed;

private:
	DirectX::XMVECTOR lookingAtVector;
	DirectX::XMFLOAT3 position;
	float nearPlane;
	float farPlane;
};