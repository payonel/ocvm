#pragma once
#include <vector>
#include <cstdint>
using std::vector;

namespace util {
    uint32_t crc32(vector<char> data);
}