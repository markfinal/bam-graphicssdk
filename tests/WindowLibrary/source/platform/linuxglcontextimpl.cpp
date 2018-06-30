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
#include "windowlibrary/glcontext.h"
#include "linuxglcontextimpl.h"
#include "windowlibrary/exception.h"

#include <GL/glx.h>

namespace WindowLibrary
{

GLContext::Impl::Impl(
    GraphicsWindow *inWindow)
    :
    _window(inWindow)
{}

GLContext::Impl::~Impl()
{
    this->destroyContext();
}

void
GLContext::Impl::createContext()
{
    auto display = this->_window->linuxDisplay();
    auto screen = this->_window->linuxScreen();
    int attributes[] =
        {
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_DEPTH_SIZE, 16,
            GLX_DOUBLEBUFFER,
            GLX_RGBA,
            None
        };
    auto visual = ::glXChooseVisual(
        display,
        screen,
        attributes
    );
    if (nullptr == visual)
    {
        throw LinuxFailedToChooseVisual();
    }
    auto context = ::glXCreateContext(display, visual, 0, True);
    if (nullptr == context)
    {
        throw LinuxFailedToCreateRenderContext();
    }
    ::XFree(visual);
}

void
GLContext::Impl::makeCurrent()
{
}

void
GLContext::Impl::detachCurrent()
{
}

void
GLContext::Impl::swapBuffers()
{
}

void
GLContext::Impl::destroyContext()
{
}

} // namespace WindowLibrary
