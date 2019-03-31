
#include "../inc/util.h"


thread_local RpsRandom* RpsRandom::_pInstance = nullptr;
thread_local std::mt19937 RpsRandom::_twister
  = std::mt19937(std::random_device{}());
thread_local uint_fast64_t RpsRandom::_counter = 0;


// TODO: this dead code will be removed if approved by Dr. Basile
#if 0
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
#endif


