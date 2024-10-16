#pragma once
#include<exception>
#include<string>
#include<sstream>
#include "dxerr.h"
#include<vector>

class MyException : public std::exception
{
public:
	MyException(int line, const char* file);
	const char* what() const;
	virtual const char* GetType() const;
	int GetLine() const;
	const std::string GetFile() const;
	std::string GetOriginString() const;

private:
	int line;
	std::string file;

protected:
	mutable std::string whatBuffer;

};

class HrException : public MyException
{
public:
	HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {});
	const char* what() const override;
	const char* GetType() const override;
	HRESULT GetErrorCode() const;
	std::string GetErrorString() const;
	std::string GetErrorDescription() const;
	std::string GetErrorInfo() const;

private:
	HRESULT hr;
	std::string info;
};
class InfoException : public MyException
{
public:
	InfoException(int line, const char* file, std::vector<std::string> infoMsgs = {});
	const char* what() const override;
	const char* GetType() const override;
	std::string GetErrorInfo() const;

private:
	std::string info;
};
class DeviceRemovedException : public HrException
{
	using HrException::HrException;
public:
	const char* GetType() const override;
};

