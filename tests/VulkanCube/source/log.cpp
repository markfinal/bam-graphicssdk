#include "log.h"

#include <Windows.h>

Log::~Log()
{
    OutputDebugString(this->_stream.str().c_str());
}

std::ostringstream &Log::get()
{
    return this->_stream;
}
