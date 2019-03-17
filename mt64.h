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
    std::cout << counter << "counter ";
    std::random_device rd;
    std::mt19937 g{ rd() };
    if (counter >= 65536 )
      g.seed(time(NULL));
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;
    std::cout << dis(gen) << ' ';
    std::cout << std::endl;
    return 0;
}