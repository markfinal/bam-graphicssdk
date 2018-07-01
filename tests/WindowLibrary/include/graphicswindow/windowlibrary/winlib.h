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
#ifndef WINDOWLIBRARY_WINLIB_H
#define WINDOWLIBRARY_WINLIB_H

#if defined(D_BAM_PLATFORM_WINDOWS)
#include "platform/win32types.h"
#elif defined(D_BAM_PLATFORM_LINUX)
#include "platform/linuxtypes.h"
#elif defined(D_BAM_PLATFORM_OSX)
#include "platform/macostypes.h"
#else
#error Unsupported platform
#endif

#include <memory>

namespace WindowLibrary
{

class GraphicsWindow
{
public:
    GraphicsWindow();
    virtual ~GraphicsWindow();

    void
    init(
        const uint32_t inWidth,
        const uint32_t inHeight);

    WindowHandle
    getNativeWindowHandle() const;

    uint32_t
    width() const;

    uint32_t
    height() const;

public:
#if defined(D_BAM_PLATFORM_WINDOWS)
    void
    win32SetInstanceHandle(
        ::HINSTANCE inInstance
    );

    LRESULT
    win32MessageProc(
        ::HWND hWnd,
        ::UINT Msg,
        ::WPARAM wParam,
        ::LPARAM lParam
    );
#elif defined(D_BAM_PLATFORM_LINUX)
    Display *
    linuxDisplay() const;

    int
    linuxScreen() const;

    Atom
    linuxDeleteWindowMessage() const;
#elif defined(D_BAM_PLATFORM_OSX)
    // TODO
#else
#error Unsupported platform
#endif // D_BAM_PLATFORM_WINDOWS

public:
    virtual void
    onCreate();

    virtual void
    onDestroy();

    virtual void
    onClose();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace WindowLibrary

#endif // WINDOWLIBRARY_WINLIB_H
