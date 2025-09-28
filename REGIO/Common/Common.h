#pragma once
#include <cstdint>
#include <string>
#include <assimp/scene.h>
#include <DirectXMath.h>

struct Vector
{
	float x;
	float y;
	float z;

	Vector(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ)
	{
	}
};

struct Vertex
{
	struct
	{
		float x;
		float y;
		float z;
	} pos;

	struct
	{
		float x;
		float y;
		float z;
	} normal;

	struct
	{
		float u;
		float v;
	} tex;

};

// This matrix wrapper is created to allow the engine to be API-agnostic 
// IMPORTANT: STORE VALUES IN ROW-MAJOR ORDER
struct Matrix4x4
{
	float m[4][4];

	DirectX::XMMATRIX ToXMMATRIX() const
	{
		return DirectX::XMMATRIX(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]
        );
	}
	// OpenGL uses matrices in column-major order
	const float* ToOpenGLPointer() const {
		float* out_m = new float[16];
		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				out_m[col * 4 + row] = m[row][col];
			}
		}
		return out_m;
    }

	Matrix4x4& operator=(const aiMatrix4x4& other)
	{
		for (int rowIndex = 0; rowIndex < 4; ++rowIndex)
		{
			for (int colIndex = 0; colIndex < 4; ++colIndex)
			{
				m[rowIndex][colIndex] = other[rowIndex][colIndex];
			}
		}
		return *this;
	}

};


struct MeshNode
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

struct EmitterNode
{
	Vector position;
	float intensity;

	EmitterNode(Vector inPosition, float inIntensity) : position(inPosition), intensity(inIntensity)
	{
	}
};

struct MaterialNode
{
	//Pointer to shaders
	uint32_t textureID;
	uint32_t shaderID;
	// In the future this will hold parameters for bsdf (I still don't know if this will be added)
};

