#pragma once

#include <chrono>

using namespace std::chrono;

typedef time_point<system_clock, nanoseconds> NanoTimePoint;

bool first_time_call = true;
NanoTimePoint *first_call_time_point;

uint64_t getNanoSeconds() {
    if (first_time_call) {
        first_time_call = false;
        first_call_time_point = static_cast<NanoTimePoint*>(std::malloc(sizeof(NanoTimePoint)));
        *first_call_time_point = system_clock::now();
    }
    auto now = system_clock::now();
    auto duration = duration_cast<nanoseconds>(now - *first_call_time_point);
    return duration.count();
}