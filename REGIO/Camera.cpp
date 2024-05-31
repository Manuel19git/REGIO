#include "Camera.h"

Camera::Camera()
{
    translationSpeed = 0.1f;
	rotationSpeed = 1.0f;

	yaw = 0.0f;
	pitch = 0.0f;


	m_orientation = DirectX::XMQuaternionIdentity();
    position = DirectX::XMFLOAT3(0.0f, 1.0f, -4.0f);
	lookAtVector = DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f);
	upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	nearPlane = 0.5f;
	farPlane = 30.0;
}

void Camera::moveCamera(Axis axis, int sign)
{
	switch (axis)
	{
	case X:
		position.x += translationSpeed * sign;
		break;
	case Y:
		position.y += translationSpeed * sign;
		break;
	case Z:
		position.z += translationSpeed * sign;
		break;
	default:
		break;
	}
}

void Camera::updateYawPitch(float x, float y)
{
	yaw = x * rotationSpeed;
	pitch = y * rotationSpeed;
}

DirectX::XMFLOAT3 Camera::getPosition()
{
	return position;
}

DirectX::XMMATRIX Camera::getTransform()
{
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMVECTOR yawQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), yaw);
	DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), pitch);

	// Combine the new rotations with the current orientation
	m_orientation = DirectX::XMQuaternionMultiply(m_orientation, pitchQuat);
	m_orientation = DirectX::XMQuaternionMultiply(yawQuat, m_orientation);

	lookAtVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f), m_orientation);
	upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

    return DirectX::XMMatrixTranspose(
		DirectX::XMMatrixLookAtLH(posVector, lookAtVector, upVector) *
		DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane)
	);

}
