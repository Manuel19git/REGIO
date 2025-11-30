// I don't know if I can put implementation in a header

#pragma once
#include <fstream>
#include <chrono>
#include <algorithm>
#include <string>

struct ProfilerResult
{
    std::string name;
    long long end;
	long long start;
};

class Profiler
{
private:
	std::ofstream mOutputStream;
    std::string mSessionName;
	int mProfileCount;
public:

    Profiler() : mProfileCount(0)
    {}

    void StartProfile(const std::string& name, const std::string& fileName = "profile.json")
    {
		mOutputStream.open(fileName);
        WriteHeader();
		mSessionName = name;
    }

    void EndProfile()
    {
        WriteFoot();
		mOutputStream.close();
        mProfileCount = 0;
    }

    void WriteProfiler(const ProfilerResult& result)
    {
        if (mProfileCount++ > 0)
            mOutputStream << ",";

        std::string name = result.name;
        std::replace(name.begin(), name.end(), '"', '\'');

        mOutputStream << "{";
        mOutputStream << "\"cat\":\"function\",";
        mOutputStream << "\"dur\":" << (result.end - result.start) << ',';
        mOutputStream << "\"name\":\"" << name << "\",";
        mOutputStream << "\"ph\":\"X\",";
        mOutputStream << "\"pid\":0,";
        mOutputStream << "\"tid\":0,"; // Thread IDs zero for now
        mOutputStream << "\"ts\":" << result.start;
        mOutputStream << "}";

        mOutputStream.flush();
    }

	void WriteHeader()
	{
		mOutputStream << "{\"otherData\": {},\"traceEvents\":[";
		mOutputStream.flush();
	}
	void WriteFoot()
	{
		mOutputStream << "]}";
		mOutputStream.flush();
	}
    
    static Profiler& Get()
    {
        static Profiler profilerInstance;
        return profilerInstance;
    }
};

class ProfilerTimer
{
public:
	ProfilerTimer(std::string funcName) : mFuncName(funcName)
	{
		mStartTimePoint = std::chrono::high_resolution_clock::now();
	}
	~ProfilerTimer()
	{
		auto endTimePoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(mStartTimePoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();
		auto duration = end - start;
		Profiler::Get().WriteProfiler({ mFuncName, end, start });
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> mStartTimePoint;
    std::string mFuncName;
};

