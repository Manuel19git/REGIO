#include "Camera.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 &startPosition, DirectX::XMVECTOR &startForward)
{
    translationSpeed = 0.3f;
	rotationSpeed = 0.005f;
	lookSensitivity = 0.02;

	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;

	m_orientation = DirectX::XMQuaternionIdentity();
	position = startPosition;
	forwardVector = startForward;
	startForwardVector = startForward;
    //position = DirectX::XMFLOAT3(0.5f, 2.0f, -4.0f);
	lookAtVector = DirectX::XMVectorSet(0.0f, 0.0f, 20.0f, 1.0f);

	//forwardVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
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
	yaw = x * lookSensitivity;
	pitch = y * lookSensitivity;
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
	return DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&position), lookAtVector, upVector);
}

DirectX::XMMATRIX Camera::getProjectionMatrix(bool isOrthographic)
{
	updateTransform();
	XMMATRIX projectionMatrix = (isOrthographic) ?
		XMMatrixOrthographicOffCenterLH(scenebbox.left, scenebbox.right, scenebbox.bottom, scenebbox.top, scenebbox.nearPlane, scenebbox.farPlane) :
		XMMatrixPerspectiveLH(1.0f, screenHeight / screenWidth, nearPlane, farPlane);

	return projectionMatrix;
}


DirectX::XMMATRIX Camera::getViewProjMatrix(bool isOrthographic)
{
	updateTransform();

    DirectX::XMMATRIX view = getViewMatrix();
    DirectX::XMMATRIX proj = getProjectionMatrix(isOrthographic);

	//DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(view * proj);

	return view * proj;
}

float Camera::getNear()
{
	return nearPlane;
}
float Camera::getFar()
{
	return farPlane;
}
void Camera::setFar(float far)
{
	farPlane = far;
}

float Camera::getYaw()
{
	return yaw;
}

float Camera::getPitch()
{
	return pitch;
}

DirectX::XMMATRIX Camera::getTransform(bool isOrthographic)
{
	// Save previous orientation to ensure camera doesn't turn around 180� when pitching (gimball lock)
	XMVECTOR prevOrientationMatrix = m_orientation;
	XMVECTOR prevForwardVector = forwardVector;

	// I want to simulate the player having a determined height
	if(!isOrthographic)
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

	//m_orientation = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

	forwardVector = DirectX::XMVector3Rotate(startForwardVector, m_orientation);
	// This condition is here to avoid the camera going up and beyond resulting in inverting the mouse controls
	if (fabs(DirectX::XMVectorGetY(forwardVector)) > 0.95f)
	{
		m_orientation = prevOrientationMatrix;
		forwardVector = prevForwardVector;
	}
	forwardVector = XMVector3Normalize(forwardVector);
	// Calculate rightVector and upVector without any limitations
	//rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	//upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

	// Calculate rightVector and upVector here to ensure right vector stays on XZ plane
	float dot = XMVectorGetX(XMVector3Dot(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));
	if (fabs(dot) > 0.999f)
		rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), forwardVector));
	else
		rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));

	upVector = XMVector3Normalize(DirectX::XMVector3Cross(forwardVector, rightVector));

	lookAtVector = XMVectorAdd(posVector, forwardVector);

	// Right now orthographic is only used for shadow maps
	XMMATRIX projectionMatrix = (isOrthographic) ?
		//XMMatrixOrthographicLH(10, 10, nearPlane, farPlane) :
		XMMatrixOrthographicOffCenterLH(scenebbox.left, scenebbox.right, scenebbox.bottom, scenebbox.top, scenebbox.nearPlane , scenebbox.farPlane ) :
		XMMatrixPerspectiveLH(1.0f, screenHeight / screenWidth, nearPlane, farPlane);

    return DirectX::XMMatrixTranspose(
		XMMatrixLookAtLH(posVector, lookAtVector, upVector) *
		//DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, nearPlane, farPlane)     //800x600 screen size
		projectionMatrix //2048x1133 screen size
	);
}
// updateTransform has to be the same as getTransform
void Camera::updateTransform(bool isOrthographic)
{
	// Save previous orientation to ensure camera doesn't turn around 180� when pitching (gimball lock)
	XMVECTOR prevOrientationMatrix = m_orientation;
	XMVECTOR prevForwardVector = forwardVector;
	
	if(!isOrthographic)
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

	forwardVector = DirectX::XMVector3Rotate(startForwardVector, m_orientation);
	// This condition is here to avoid the camera going up and beyond resulting in inverting the mouse controls
	if (fabs(DirectX::XMVectorGetY(forwardVector)) > 0.95f)
	{
		m_orientation = prevOrientationMatrix;
		forwardVector = prevForwardVector;
	}
	forwardVector = XMVector3Normalize(forwardVector);
	// Calculate rightVector and upVector without any limitations
	//rightVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), m_orientation);
	//upVector = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), m_orientation);

	// Calculate rightVector and upVector here to ensure right vector stays on XZ plane
	float dot = XMVectorGetX(XMVector3Dot(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));
	if (fabs(dot) > 0.999f)
		rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), forwardVector));
	else
		rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), forwardVector));

	upVector = XMVector3Normalize(DirectX::XMVector3Cross(forwardVector, rightVector));

	lookAtVector = XMVectorAdd(posVector, forwardVector);
}

BoundingBox Camera::getSceneBBox()
{
	return scenebbox; 
}
void Camera::setSceneBBox(BoundingBox& bbox)
{
	scenebbox = bbox;
}
