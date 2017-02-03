#include "shell.h"
#include "iframe.h"

Shell::Shell()
{
}

Shell::~Shell()
{
    close();
}

bool Shell::add(IFrame* pframe)
{
    return false;
}

bool Shell::update()
{
    return false;
}

void Shell::close()
{
}
