#pragma once
#define NOMINMAX
#include <Windows.h>

struct delata_time_util
{
private:
    // Global variables
    DWORD prevTime = 0;
    
    float delta_time = 1.0f/60.0f;

public:

    delata_time_util()
    {
        prevTime = GetTickCount();
    }

    // Function to calculate delta time
    float update_delta_time()
    {
        DWORD currentTime = GetTickCount();
        float deltaTime = (currentTime - prevTime) / 1000.0f; // Convert milliseconds to seconds
        prevTime = currentTime;
        return deltaTime;
    }

    float get_delta_time()
    {
        return delta_time;
    }
};