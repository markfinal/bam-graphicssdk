/*
Copyright (c) 2010-2019, Mark Final
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of BuildAMation nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "log.h"

#ifdef D_BAM_PLATFORM_WINDOWS
#include <Windows.h>
#else
#include <iostream>
#endif

#include <fstream>
#include <stdexcept>

std::string   Log::_filelog_path;
std::ofstream Log::_filelog;

Log::~Log()
{
    auto message = this->_stream.str();
#ifdef D_BAM_PLATFORM_WINDOWS
    OutputDebugString(message.c_str());
#else
    std::cout << message;
#endif
    _filelog.open(_filelog_path.c_str(), std::ios_base::out | std::ios_base::app);
    if (_filelog)
    {
        _filelog << message;
        _filelog.close();
    }
}

void
Log::set_path(
    const std::string &inPath)
{
    _filelog_path = inPath;
    _filelog.open(_filelog_path.c_str(), std::ios_base::out);
    if (!_filelog)
    {
        throw std::runtime_error("Unable to open log file");
    }
    _filelog.close();
}

std::ostringstream &Log::get()
{
    return this->_stream;
}
