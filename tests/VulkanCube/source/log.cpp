#include "log.h"

#ifdef D_BAM_PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <iostream>
#endif

Log::~Log()
{
#ifdef D_BAM_PLATFORM_WINDOWS
    OutputDebugString(this->_stream.str().c_str());
#else
    std::cout << this->_stream.str();
#endif
}

std::ostringstream &Log::get()
{
    return this->_stream;
}
