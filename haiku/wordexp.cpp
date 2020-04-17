#include "wordexp.h"

bool wordexp(const char* cpath, wordexp_t* pWord, int flags)
{
  pWord->we_wordc = 0;
  pWord->we_wordv.clear();

  if (cpath == nullptr)
    return false;

  if (cpath[0] == '~')
  {
    static std::string home = "/boot/home";
    std::string right(static_cast<const char*>(cpath + 1));
    pWord->we_wordv.push_back(home + right);
    pWord->we_wordc++;
  }

  return true;
}

void wordfree(wordexp_t* pWord)
{
  // no op
}
