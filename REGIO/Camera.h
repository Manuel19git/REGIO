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
	void updateYawPitch(float x, float y);
	void updateRoll(int sign);

	DirectX::XMFLOAT3 getPosition();
	DirectX::XMVECTOR getLookAt();
	DirectX::XMVECTOR getForward();
	DirectX::XMVECTOR getRight();
	DirectX::XMVECTOR getUp();
	DirectX::XMMATRIX getViewMatrix();
	DirectX::XMMATRIX getProjectionMatrix();
	float getNear();
	float getFar();
	DirectX::XMMATRIX getTransform();
	void updateTransform();

public:
	float translationSpeed;
	float rotationSpeed;

private:
	DirectX::XMVECTOR m_orientation;
	DirectX::XMVECTOR upVector;
	DirectX::XMVECTOR lookAtVector;
	DirectX::XMFLOAT3 position;

	DirectX::XMVECTOR forwardVector;
	DirectX::XMVECTOR rightVector;

	float yaw;
	float pitch;
	float roll;

	float nearPlane;
	float farPlane;
};