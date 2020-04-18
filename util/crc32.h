#pragma once
#include <cstdint>
#include <vector>
using std::vector;

namespace util
{
uint32_t crc32(vector<char> data);
}