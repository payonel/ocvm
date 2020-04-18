#include "md5.h"

#include <cstdint>

using std::vector;

// clang-format off
static uint32_t s[64] =
{
  7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
  5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

static uint32_t K[64] =
{
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
  0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
  0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
  0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
  0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
  0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};
// clang-format on

vector<char> util::md5(vector<char> message)
{
  auto originalLength = (message.size() * 8) % 0x8000000000000000;

  message.push_back(0x80);
  while (message.size() % 64 != 56)
  {
    message.push_back(0x00);
  }

  message.push_back(originalLength & 0xFF);
  message.push_back((originalLength >> 8) & 0xFF);
  message.push_back((originalLength >> 16) & 0xFF);
  message.push_back((originalLength >> 24) & 0xFF);
  message.push_back((originalLength >> 32) & 0xFF);
  message.push_back((originalLength >> 40) & 0xFF);
  message.push_back((originalLength >> 48) & 0xFF);
  message.push_back((originalLength >> 56) & 0x8F);

  uint32_t a0 = 0x67452301;
  uint32_t b0 = 0xefcdab89;
  uint32_t c0 = 0x98badcfe;
  uint32_t d0 = 0x10325476;

  for (uint32_t i = 0; i < message.size(); i += 64)
  {
    vector<uint32_t> words{};
    for (uint32_t j = i; j < i + 64; j += 4)
    {
      words.push_back(((message[j] << 24) & (message[j + 1] << 16) & (message[j + 2] << 8) & message[j + 3]) % 0x80000000);
    }

    uint32_t A = a0;
    uint32_t B = b0;
    uint32_t C = c0;
    uint32_t D = d0;

    for (uint32_t j = 0; j < 64; j++)
    {
      uint32_t F, g;

      if (j < 15)
      {
        F = (B & C) | ((~B) & D);
        g = j;
      }
      else if (j < 32)
      {
        F = (D & B) | ((~D) & C);
        g = (5 * j + 1) % 16;
      }
      else if (j < 48)
      {
        F = B ^ C ^ D;
        g = (3 * j + 5) % 16;
      }
      else
      {
        F = C ^ (B | (~D));
        g = (7 * j) % 16;
      }

      F += A + K[j] + words[g];
      A = D;
      D = C;
      C = B;
      B += ((F << s[j]) | (F >> s[j]));
    }

    a0 = (a0 + A) % 0x80000000;
    b0 = (b0 + B) % 0x80000000;
    c0 = (c0 + C) % 0x80000000;
    d0 = (d0 + D) % 0x80000000;
  }

  return vector<char>{
    (char)(a0),
    (char)((a0 >> 8) & 0xFF),
    (char)((a0 >> 16) & 0xFF),
    (char)((a0 >> 24) & 0xFF),

    (char)(b0),
    (char)((b0 >> 8) & 0xFF),
    (char)((b0 >> 16) & 0xFF),
    (char)((b0 >> 24) & 0xFF),

    (char)(c0),
    (char)((c0 >> 8) & 0xFF),
    (char)((c0 >> 16) & 0xFF),
    (char)((c0 >> 24) & 0xFF),

    (char)(d0),
    (char)((d0 >> 8) & 0xFF),
    (char)((d0 >> 16) & 0xFF),
    (char)((d0 >> 24) & 0xFF),
  };
}
