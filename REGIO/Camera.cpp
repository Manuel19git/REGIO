#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
    translationSpeed = 0.1f;
	rotationSpeed = 0.005f;

	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;

	m_orientation = DirectX::XMQuaternionIdentity();
    position = DirectX::XMFLOAT3(0.5f, 2.0f, -4.0f);
	lookAtVector = DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f);

	forwardVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	rightVector = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	nearPlane = 0.5f;
	farPlane = 10000.0;

	screenWidth = 0.0f;
	screenHeight = 0.0f;
}

void Camera::moveCamera(Axis axis, int sign)
{
	XMVECTOR posVector;
	switch (axis)
	{
	case X:
		posVector = XMLoadFloat3(&position);
		posVector += rightVector * translationSpeed * sign;
		XMStoreFloat3(&position, posVector);
		break;
	case Y:
		posVector = XMLoadFloat3(&position);
		posVector += upVector * translationSpeed * sign;
		XMStoreFloat3(&position, posVector);
		break;
	case Z:
		posVector = XMLoadFloat3(&position);
		posVector += forwardVector * translationSpeed * sign;
		XMStoreFloat3(&position, posVector);
		break;
	default:
		break;
	}
}

void Camera::updateYawPitch(float x, float y)
{
	float speed = (screenWidth < 2000 && screenHeight < 1000) ? 0.6f : 2.0f;
	float error = 0.0035f; //Sweet spot
	if (abs(x) > error )
		yaw = x * speed;
	else
		yaw = 0;
	if (abs(y) > error)
		pitch = y * speed;
	else
		pitch = 0;
}

void Camera::setResolution(const float width, const float height)
{
	screenWidth = width;
	screenHeight = height;
}

std::pair<float, float> Camera::getResolution()
{
	return std::make_pair(screenWidth, screenHeight);
}


void Camera::updateRoll(int sign)
{
	roll += rotationSpeed * sign;
}

DirectX::XMFLOAT3 Camera::getPosition()
{
	return position;
}

DirectX::XMVECTOR Camera::getLookAt()
{
	return lookAtVector;
}

DirectX::XMVECTOR Camera::getForward()
{
	return forwardVector;
}

DirectX::XMVECTOR Camera::getRight()
{
	return rightVector;
}
DirectX::XMVECTOR Camera::getUp()
{
	return upVector;
}

DirectX::XMMATRIX Camera::getViewMatrix()
{
	updateTransform();
	XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&position), lookAtVector, upVector);
	viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	return viewMatrix;
}

DirectX::XMMATRIX Camera::getProjectionMatrix()
{
	updateTransform();
	return DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane);
}

float Camera::getNear()
{
	return nearPlane;
}
float Camera::getFar()
{
	return farPlane;
}

DirectX::XMMATRIX Camera::getTransform()
{
	// Save previous orientation to ensure camera doesn't turn around 180� when pitching (gimball lock)
	XMVECTOR prevOrientationMatrix = m_orientation;
	XMVECTOR prevForwardVector = forwardVector;

	position.y = 3.0f;
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMVECTOR yawQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), yaw);
	DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), pitch);
	DirectX::XMVECTOR rollQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), roll);
	yaw = 0.0;
	pitch = 0.0;
	roll = 0.0;

	// Combine the new rotations with the current orientation
	m_orientation = DirectX::XMQuaternionMultiply(pitchQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(yawQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(rollQuat, m_orientation);

	forwardVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), m_orientation);
	if (fabs(DirectX::XMVectorGetY(forwardVector)) > 0.95f)
	{
		m_orientation = prevOrientationMatrix;
		forwardVector = prevForwardVector;
	}

	// Calculate rightVector and upVector without any limitations
	//rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	//upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

	// Calculate rightVector and upVector here to ensure right vector stays on XZ plane
	rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));
	upVector = DirectX::XMVector3Cross(forwardVector, rightVector);

	lookAtVector = XMVectorAdd(posVector, forwardVector);

    return DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(posVector, lookAtVector, upVector) *
		//DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane)     //800x600 screen size
		DirectX::XMMatrixPerspectiveLH(1.0f, screenHeight / screenWidth, nearPlane, farPlane) //2048x1133 screen size
	);
}
// updateTransform has to be the same as getTransform
void Camera::updateTransform()
{
	// Save previous orientation to ensure camera doesn't turn around 180� when pitching (gimball lock)
	XMVECTOR prevOrientationMatrix = m_orientation;
	XMVECTOR prevForwardVector = forwardVector;

	position.y = 3.0f;
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMVECTOR yawQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), yaw);
	DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), pitch);
	DirectX::XMVECTOR rollQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), roll);
	roll = 0.0;

	// Combine the new rotations with the current orientation
	m_orientation = DirectX::XMQuaternionMultiply(pitchQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(yawQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(rollQuat, m_orientation);

	forwardVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), m_orientation);
	if (fabs(DirectX::XMVectorGetY(forwardVector)) > 0.95f)
	{
		m_orientation = prevOrientationMatrix;
		forwardVector = prevForwardVector;
	}
	// Calculate rightVector and upVector without any limitations
	//rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	//upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

	// Calculate rightVector and upVector here to ensure right vector stays on XZ plane
	rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));
	upVector = DirectX::XMVector3Cross(forwardVector, rightVector);

	lookAtVector = XMVectorAdd(posVector, forwardVector);
}
