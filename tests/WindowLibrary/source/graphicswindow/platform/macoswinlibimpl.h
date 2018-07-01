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
#ifndef WINDOWLIBRARY_WINLIB_PLATFORM_MACOSWINLIBIMPL_H
#define WINDOWLIBRARY_WINLIB_PLATFORM_MACOSWINLIBIMPL_H

#include "windowlibrary/graphicswindow.h"

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

namespace WindowLibrary
{

struct GraphicsWindow::Impl
{
    GraphicsWindow *_parent = nullptr;
    uint32_t        _width;
    uint32_t        _height;
#ifdef __OBJC__
    NSWindow *_window = nullptr;
#else
    void     *_window = nullptr;
#endif

    Impl(
        GraphicsWindow *inParent);
    ~Impl();

    void
    createWindow(
        const uint32_t inWidth,
        const uint32_t inHeight,
        const std::string &inTitle);

    void
    show();

private:
    void
    destroyWindow();
};

} // namespace WindowLibrary

#endif // WINDOWLIBRARY_WINLIB_PLATFORM_MACOSWINLIBIMPL_H
