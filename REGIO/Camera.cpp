#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
    translationSpeed = 0.5f;
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
	yaw = x;
	pitch = y;
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
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMVECTOR yawQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), yaw);
	DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), pitch);
	DirectX::XMVECTOR rollQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), roll);
	roll = 0.0;

	// Combine the new rotations with the current orientation
	m_orientation = DirectX::XMQuaternionMultiply(pitchQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(yawQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(rollQuat, m_orientation);

	lookAtVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f), m_orientation);
	lookAtVector += posVector;

	forwardVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), m_orientation);
	rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

    return DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(posVector, lookAtVector, upVector) *
		DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane)
	);
}

void Camera::updateTransform()
{
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMVECTOR yawQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), yaw);
	DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), pitch);
	DirectX::XMVECTOR rollQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), roll);
	roll = 0.0;

	// Combine the new rotations with the current orientation
	m_orientation = DirectX::XMQuaternionMultiply(pitchQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(yawQuat, m_orientation);
	m_orientation = DirectX::XMQuaternionMultiply(rollQuat, m_orientation);

	lookAtVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f), m_orientation);
	lookAtVector += posVector;

	forwardVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), m_orientation);
	rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);
}
