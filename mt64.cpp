#include <random>
#include <iostream>
int main()
{
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<unsigned long long> dis;
  std::cout << dis(gen) << ' ';
  std::cout << std::endl;
  return 0;
}