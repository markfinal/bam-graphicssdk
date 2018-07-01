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
#include "win32glcontextimpl.h"
#include "windowlibrary/exception.h"

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
    auto window_handle = this->_window->getNativeWindowHandle();
    ::HDC hDC = ::GetDC(window_handle);
    ::PIXELFORMATDESCRIPTOR pfDescriptor;
    ::ZeroMemory(&pfDescriptor, sizeof(pfDescriptor));
    pfDescriptor.nSize = sizeof(pfDescriptor);
    pfDescriptor.nVersion = 1;
    DWORD flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfDescriptor.dwFlags = flags;
    pfDescriptor.iPixelType = PFD_TYPE_RGBA;
    pfDescriptor.cColorBits = 8;
    //pfDescriptor.cDepthBits = 24;
    //pfDescriptor.cStencilBits = 8;

    int pixelFormat = ::ChoosePixelFormat(hDC, &pfDescriptor);
    if (0 == pixelFormat)
    {
        throw Win32FailedToChoosePixelFormat();
    }

    BOOL result = ::SetPixelFormat(hDC, pixelFormat, &pfDescriptor);
    if (FALSE == result)
    {
        throw Win32FailedToSetPixelFormat();
    }

    ::HGLRC hRC = ::wglCreateContext(hDC);
    if (0 == hRC)
    {
        throw Win32FailedToCreateRenderContext();
    }

    ::ReleaseDC(window_handle, hDC);

    this->_dc = hDC;
    this->_rc = hRC;
}

void
GLContext::Impl::makeCurrent()
{
    // set the current RC in this thread
    this->do_make_current(this->_rc);
}

void
GLContext::Impl::detachCurrent()
{
    // set no RC in this thread
    this->do_make_current(nullptr);
}

void
GLContext::Impl::swapBuffers()
{
    ::SwapBuffers(this->_dc);
}

void
GLContext::Impl::destroyContext()
{
    ::BOOL result = ::wglDeleteContext(this->_rc);
    if (FALSE == result)
    {
        throw Win32FailedToDeleteRenderContext();
    }

    auto window_handle = this->_window->getNativeWindowHandle();
    ::ReleaseDC(window_handle, this->_dc);
}

void
GLContext::Impl::do_make_current(
    ::HGLRC inRenderContext)
{
    BOOL result = ::wglMakeCurrent(this->_dc, inRenderContext);
    if (FALSE == result)
    {
        throw Win32FailedToMakeRenderContextCurrent();
    }
}

} // namespace WindowLibrary
