#pragma once
#include <chrono>
#include <vector>
#include <iostream>

struct Timings
{
    double keygen   = 0;
    double encrypt  = 0;
    double decrypt  = 0;
    double add      = 0;
    double mult     = 0;
};

inline double usNow()
{
    using clk = std::chrono::high_resolution_clock;
    return std::chrono::duration<double, std::micro>(
               clk::now().time_since_epoch()).count();
}

template<typename F>
double benchRepeated(F func, long reps = 10)
{
    double t0 = usNow();
    for (long i = 0; i < reps; ++i) func();
    return (usNow() - t0) / reps;
}
