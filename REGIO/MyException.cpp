#include "MyException.h"

MyException::MyException(int line, const char* file) : line(line), file(file) {}

const char* MyException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
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

std::string MyException::GetOriginString() const
{
	std::ostringstream oss;
	oss << "[File]" << file << std::endl
		<< "[Line]" << line;
	return oss.str();
}

//-------------------------------------------------------------------------------------------------------------
//Here we implement hr exceptions
HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs)
    :
    MyException(line, file),
    hr(hr)
{
    //join messages
    for (const auto& m : infoMsgs)
    {
        info += m;
        info.push_back('\n');
    }
    //remove final line if exists
    if (!info.empty())
    {
        info.pop_back();
    }
}

const char* HrException::what() const
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << "(" << (unsigned long)GetErrorCode() << ")" << std::endl
        << "[Error String] " << GetErrorString() << std::endl
        << "[Error Description] " << GetErrorDescription() << std::endl;
    if (!info.empty())
    {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* HrException::GetType() const
{
    return "My Graphics Exception";
}

HRESULT HrException::GetErrorCode() const
{
    return hr;
}

std::string HrException::GetErrorString() const
{
    return DXGetErrorStringA(hr);
}

std::string HrException::GetErrorDescription() const
{
    char buf[512];
    DXGetErrorDescriptionA(hr, buf, sizeof(buf));
    return buf;
}

std::string HrException::GetErrorInfo() const
{
    return info;
}

//-------------------------------------------------------------------------------------------------------------
InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs)
    :
    MyException(line, file)
{
    //join messages
    for (const auto& m : infoMsgs)
    {
        info += m;
        info.push_back('\n');
    }
    //remove final line if exists
    if (!info.empty())
    {
        info.pop_back();
    }
}

const char* InfoException::what() const
{
    std::ostringstream oss;
    oss << GetType() << std::endl;
    if (!info.empty())
    {
        oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl;
    }
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* InfoException::GetType() const
{
    return "Output Info Only Exception";
}

std::string InfoException::GetErrorInfo() const
{
    return info;
}

//-------------------------------------------------------------------------------------------------------------
const char* DeviceRemovedException::GetType() const
{
    return "My Graphics Exception [Device Removed] DXGI_ERROR_DEVICE_REMOVED";
}


