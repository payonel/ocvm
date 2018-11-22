#pragma once

#include <vector>
#include <string>

struct wordexp_t
{
    int we_wordc;
    std::vector<std::string> we_wordv;
};

bool wordexp(const char* cpath, wordexp_t* pWord, int flags);
void wordfree(wordexp_t* pWord);
