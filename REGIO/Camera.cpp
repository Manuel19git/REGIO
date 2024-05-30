#include "Camera.h"

Camera::Camera()
{
    translationSpeed = 0.1f;
    position = DirectX::XMFLOAT3(0.0f, 1.0f, -4.0f);
	lookingAtVector = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);

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


DirectX::XMFLOAT3 Camera::getPosition()
{
	return position;
}

DirectX::XMMATRIX Camera::getTransform()
{
	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR cameraDirection = DirectX::XMVectorSubtract(lookingAtVector, posVector);

    return DirectX::XMMatrixTranspose(
		DirectX::XMMatrixTranslation(-position.x, -position.y, -position.z) *
		DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane)
	);

}
