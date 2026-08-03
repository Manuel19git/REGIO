#include "Common.h"


std::vector<std::string> commonSearchDirectories;

std::string getDirectory(std::string filePath, char separator)
{
	const size_t idx = filePath.rfind(separator);
	if (std::string::npos != idx)
	{
		return filePath.substr(0, idx);
	}
	
	return "";
}

std::string getExtension(std::string filePath)
{
	const size_t idx = filePath.rfind('.');
	if (std::string::npos != idx)
	{
		return filePath.substr(idx);
	}
	
	return "";
}

std::string searchFile(std::string path)
{
	// We don't need to search if path is allready a full path
	std::ifstream file(path);
	if (file.good())
	{
		return path;
	}
	
	// Check for unix separators
	char separator = '\\';
	if (path.rfind('/') != std::string::npos)
		separator = '/';
	
	// First we check for common directories if there are any	
	for (int i = 0; i < commonSearchDirectories.size(); ++i)
	{
		std::string fullPath = commonSearchDirectories[i] + separator + path;
		std::ifstream file(fullPath);
		if (file.good())
		{
			return fullPath;
		}
	}

	// Then we check on parent directories
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string modulePath = std::string(buffer);
	std::string moduleDir = modulePath.substr(0, modulePath.find_last_of("\\/"));

	while (moduleDir != "C:")
	{
		std::string fullPath = moduleDir + path;
		std::ifstream file(fullPath);
		if (file.good())
		{
			return fullPath;
		}
		moduleDir = moduleDir.substr(0, moduleDir.find_last_of("\\/"));
	}
	return "";
}
