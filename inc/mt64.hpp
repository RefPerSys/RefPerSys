#include <random>
#include <iostream>
#include <ctime>

extern "C" {
   uint64_t rps_random_uint64(void);
}
static thread_local int counter = 0;
uint64_t rps_random_uint64(void)
{
    counter++;
    static thread_local std::random_device rd;
    static thread_local std::mt19937 g(std::random_device{}());
    if (counter % 65536 == 0)
      static thread_local std::mt19937 g(std::random_device{}());
    static thread_local std::mt19937_64 gen(rd());
    static thread_local std::uniform_int_distribution<unsigned long long> dis;
    return dis(gen);
}