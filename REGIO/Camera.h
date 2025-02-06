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


struct BoundingBox // TODO put this in a separate file with the rest of auxiliar stuff
{
	float left;
	float right;
	float top;
	float bottom;
	float nearPlane;
	float farPlane;
};

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 &startPosition, DirectX::XMVECTOR &startForward, BoundingBox bbox);

	void moveCamera(Axis axis, int sign);
	void updateYawPitch(float x, float y);
	void updateRoll(int sign);
	void setResolution(const float width, const float height);
	std::pair<float, float> getResolution();

	DirectX::XMFLOAT3 getPosition();
	DirectX::XMVECTOR getLookAt();
	DirectX::XMVECTOR getForward();
	DirectX::XMVECTOR getRight();
	DirectX::XMVECTOR getUp();
	DirectX::XMMATRIX getViewMatrix();
	DirectX::XMMATRIX getProjectionMatrix(bool isOrthographic = false);
	float getNear();
	float getFar();
	float getYaw();
	float getPitch();
	DirectX::XMMATRIX getTransform(bool isOrthographic = false);
	void updateTransform(bool isOrthographic = false);

public:
	float translationSpeed;
	float rotationSpeed;

private:
	DirectX::XMVECTOR m_orientation;
	DirectX::XMVECTOR lookAtVector;
	DirectX::XMFLOAT3 position;

	DirectX::XMVECTOR startForwardVector;
	DirectX::XMVECTOR forwardVector;
	DirectX::XMVECTOR rightVector;
	DirectX::XMVECTOR upVector;

	float yaw;
	float pitch;
	float roll;

	float nearPlane;
	float farPlane;
	float screenWidth;
	float screenHeight;
	BoundingBox scenebbox;
};