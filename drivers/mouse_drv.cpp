#include "mouse_drv.h"

void MouseDriverImpl::enqueue()
{
    _source->push({});
}
