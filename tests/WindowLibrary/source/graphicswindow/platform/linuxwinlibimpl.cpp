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
#include "windowlibrary/graphicswindow.h"
#include "windowlibrary/exception.h"
#include "linuxwinlibimpl.h"

#include <cassert>

namespace WindowLibrary
{

GraphicsWindow::Impl::Impl(
    GraphicsWindow *inParent)
    :
    _parent(inParent)
{
    ::XInitThreads();
}

GraphicsWindow::Impl::~Impl()
{
    this->destroyWindow();
}

void
GraphicsWindow::Impl::createWindow(
    const uint32_t inWidth,
    const uint32_t inHeight,
    const std::string &inTitle)
{
    char *displayName = nullptr;
    auto display = ::XOpenDisplay(displayName);
    if (nullptr == display)
    {
        throw LinuxFailedToOpenDisplay();
    }

    auto screen = DefaultScreen(display);

    auto window = ::XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0, 0,
        inWidth, inHeight,
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    ::XStoreName(display, window, inTitle.c_str());
    ::XSelectInput(display, window, ExposureMask | KeyPressMask);

    // register interest in the delete window message
    auto wmDeleteMessage = ::XInternAtom(display, "WM_DELETE_WINDOW", False);
    ::XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    this->_screen = screen;
    this->_display = display;
    this->_window = window;
    this->_deleteWindowMessage = wmDeleteMessage;
    this->_width = inWidth;
    this->_height = inHeight;

    this->_parent->onCreate();
}

void
GraphicsWindow::Impl::show()
{
    ::XMapWindow(this->_display, this->_window);
}

void
GraphicsWindow::Impl::destroyWindow()
{
    // TODO
}

} // namespace WindowLibrary
