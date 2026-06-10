#pragma once
#include "assimp/matrix4x4.h"
#include <fstream>
#include <cstdint>
#include <string>
#include <assimp/scene.h>
#include <DirectXMath.h>

// Conversion widestring string
#include<locale>
#include<codecvt>

#include <Windows.h>
#include <vector>

struct Vector
{
	float x;
	float y;
	float z;
	float w;

	Vector(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW)
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

	Matrix4x4()
	{
		// Initialize as an Identity Matrix by default
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
	}
	
	Matrix4x4(const aiMatrix4x4& other)
	{
		for (int rowIndex = 0; rowIndex < 4; ++rowIndex)
		{
			for (int colIndex = 0; colIndex < 4; ++colIndex)
			{
				m[rowIndex][colIndex] = other[rowIndex][colIndex];
			}
		}
	}

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

	Matrix4x4& Transpose()
	{
		float temp;
		// Swap (0,1) with (1,0)
		temp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = temp;
		// Swap (0,2) with (2,0)
		temp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = temp;
		// Swap (0,3) with (3,0)
		temp = m[0][3]; m[0][3] = m[3][0]; m[3][0] = temp;
    
		// Swap (1,2) with (2,1)
		temp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = temp;
		// Swap (1,3) with (3,1)
		temp = m[1][3]; m[1][3] = m[3][1]; m[3][1] = temp;
    	// Swap (2,3) with (3,2)
		temp = m[2][3]; m[2][3] = m[3][2]; m[3][2] = temp;

		return *this;
	}

	// Overload subscript to access rows: myMatrix[row][col]
    float* operator[](int index) {
        return m[index];
    }

    const float* operator[](int index) const {
        return m[index];
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

	Matrix4x4 operator*(Matrix4x4& other)
	{
		Matrix4x4 result;
		for (int rowIndex = 0; rowIndex < 4; ++rowIndex)
		{
			for (int colIndex = 0; colIndex < 4; ++colIndex)
			{
				result[rowIndex][colIndex] = m[rowIndex][0] * other[0][colIndex] +
										m[rowIndex][1] * other[1][colIndex] +
										m[rowIndex][2] * other[2][colIndex] +
										m[rowIndex][3] * other[3][colIndex];
			}
		}
		
		return result;
	}

};

// File Related functions

extern std::vector<std::string> commonSearchDirectories; // This variable is shared for every cpp that includes this header
std::string getDirectory(std::string filePath, char separator = '\\');
std::string getExtension(std::string filePath);
std::string searchFile(std::string path);


// Function to transform regular string to wide string 
inline std::wstring string2WideString(const std::string& s)
{
	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(s);
}

inline std::string wideString2String(const std::wstring& s)
{
	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.to_bytes(s);
}

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
	std::string name;

	//Pointer to shaders
	std::string normalTexturePath = "";
	std::string diffuseTexturePath = "";
	std::string specularTexturePath = "";
	std::string vertexShaderPath = searchFile("\\shaders\\VertexShader.cso");
	std::string pixelShaderPath = searchFile("\\shaders\\PixelShader.cso");
	// In the future this will hold parameters for bsdf (I still don't know if this will be added)
	Vector ambient = Vector(1,1,1,1);
	Vector diffuse = Vector(1,1,1,1);
	Vector specular = Vector(0.2,0.2,0.2,1.0);
	Vector reflect = Vector(0,0,0,1);
};
