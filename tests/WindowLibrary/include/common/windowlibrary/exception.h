/*
Copyright (c) 2010-2018, Mark Final
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
#ifndef WINDOWLIBRARY_EXCEPTION_H
#define WINDOWLIBRARY_EXCEPTION_H

#include <exception>

#ifdef D_BAM_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace WindowLibrary
{

#if defined(D_BAM_PLATFORM_WINDOWS)
class Win32BaseException :
    public std::exception
{
protected:
    Win32BaseException();

protected:
    ::DWORD _error_code;
};

class Win32FailedToRegisterClass final :
    public Win32BaseException
{};

class Win32FailedToUnregisterClass final :
    public Win32BaseException
{};

class Win32FailedToCreateWindow final :
    public Win32BaseException
{};

class Win32FailedToDestroyWindow final :
    public Win32BaseException
{};

class Win32FailedToChoosePixelFormat final :
    public Win32BaseException
{};

class Win32FailedToSetPixelFormat final :
    public Win32BaseException
{};

class Win32FailedToCreateRenderContext final :
    public Win32BaseException
{};

class Win32FailedToDeleteRenderContext final :
    public Win32BaseException
{};

class Win32FailedToMakeRenderContextCurrent final :
    public Win32BaseException
{};
#elif defined(D_BAM_PLATFORM_LINUX)
class LinuxBaseException :
    public std::exception
{};

class LinuxFailedToOpenDisplay :
    public LinuxBaseException
{};

class LinuxFailedToChooseVisual :
    public LinuxBaseException
{};

class LinuxFailedToCreateRenderContext :
    public LinuxBaseException
{};

class LinuxFailedToMakeRenderContextCurrent :
    public LinuxBaseException
{};
#endif // D_BAM_PLATFORM_WINDOWS

} // namespace WindowLibrary

#endif // WINDOWLIBRARY_EXCEPTION_H
