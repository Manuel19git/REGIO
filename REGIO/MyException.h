#pragma once
#include<exception>
#include<string>
#include<sstream>
#include "dxerr.h"

class MyException : public std::exception
{
public:
	MyException(int line, const char* file);
	const char* what() const;
	virtual const char* GetType() const;
	int GetLine() const;
	const std::string GetFile() const;
	std::string GetOriginstring() const;

private:
	int line;
	std::string file;

protected:
	mutable std::string whatBuffer;

};