#include "MyException.h"

MyException::MyException(int line, const char* file) : line(line), file(file) {}

const char* MyException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginstring();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* MyException::GetType() const
{
	return "My Exception";
}

int MyException::GetLine() const
{
	return line;
}

const std::string MyException::GetFile() const
{
	return file;
}

std::string MyException::GetOriginstring() const
{
	std::ostringstream oss;
	oss << "[File]" << file << std::endl
		<< "[Line]" << line;
	return oss.str();
}