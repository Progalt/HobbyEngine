#pragma once
#include <chrono>

class Timer
{
public:
    void Start()
    {
        mStartTime = std::chrono::system_clock::now();
        mRunning = true;
    }

    void Stop()
    {
        mEndTime = std::chrono::system_clock::now();
        mRunning = false;
    }

    double ElapsedMilliseconds()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime;

        if (mRunning)
        {
            endTime = std::chrono::system_clock::now();
        }
        else
        {
            endTime = mEndTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - mStartTime).count();
    }

    double ElapsedSeconds()
    {
        return ElapsedMilliseconds() / 1000.0;
    }

    double ElapsedMicroseconds()
    {
        return ElapsedMilliseconds() * 1000.0;
    }

private:
    std::chrono::time_point<std::chrono::system_clock> mStartTime;
    std::chrono::time_point<std::chrono::system_clock> mEndTime;
    bool mRunning = false;
};