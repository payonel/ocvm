#include "utils.h"

#include <fstream>
using std::ifstream;
using std::ofstream;

bool utils::read(const string& path, string* pOutData)
{
    ifstream file;
    file.open(path);
    if (!file)
        return false;

    // read without p is just open test
    if (pOutData)
    {
        pOutData->clear();

        char ch;
        while (true)
        {
            ch = file.get();
            if (!file)
                break;
            (*pOutData) += ch;
        }
    }

    file.close();
    return true;
}

bool utils::copy(const string& src, const string& dst)
{
    string buffer;
    if (!read(src, &buffer))
        return false;

    return utils::write(buffer, dst);
}

bool utils::write(const string& data, const string& dst)
{
    ofstream file;
    file.open(dst);
    if (!file)
        return false;
    
    file << data;
    file.close();
    return true;
}
