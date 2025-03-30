#pragma once

#include <WinBase.h>
#include <DirectXMath.h>

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT4 Reflect;
};

struct DirectionalLight
{
	//Zero memory initializes to zero this struct
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;
	float Intensity;
	
};

struct PointLight
{
	//Zero memory initializes to zero this struct
	PointLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Att;
	float Intensity;
};

struct SpotLight
{
	//Zero memory initializes to zero this struct
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Direction;
	float Spot;

	DirectX::XMFLOAT3 Att;
	float Intensity;
};

struct cbPerObject
{
	DirectX::XMMATRIX gTransform;
	DirectX::XMMATRIX gTransformSun;
	Material gMaterial;
	int hasTexture;
};

struct cbPerFrame
{
	DirectionalLight gDirLight;
    PointLight gPointLights[6];
    SpotLight gSpotLight;
	DirectX::XMFLOAT3 gEyePosW;
	float padding;
};