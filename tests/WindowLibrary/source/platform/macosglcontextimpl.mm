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
#include "macosglcontextimpl.h"
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
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize    , 24                           ,
        NSOpenGLPFAAlphaSize    , 8                            ,
        NSOpenGLPFADoubleBuffer ,
        NSOpenGLPFAAccelerated  ,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];
    assert(nullptr != pixelFormat);
    auto window = this->_window->getNativeWindowHandle();
    assert(nullptr != window);
    this->_view = [[NSOpenGLView alloc] initWithFrame:[[window contentView] bounds] pixelFormat:pixelFormat];
    assert(nullptr != this->_view);
    [pixelFormat release];
    [[window contentView] addSubview:this->_view];
    // TODO: trying this https://stackoverflow.com/questions/20083027/nsopenglview-and-cvdisplaylink-no-default-frame-buffer
    //[[this->_view openGLContext] setView:this->_view];

    // TODO: can only invoke prepareOpenGL in the main thread
    //[[this->_view openGLContext] makeCurrentContext];
    //[this->_view prepareOpenGL];
    //[this->_view clearGLContext];
}

void
GLContext::Impl::makeCurrent()
{
    [[this->_view openGLContext] makeCurrentContext];
}

void
GLContext::Impl::detachCurrent()
{
    [this->_view clearGLContext];
}

void
GLContext::Impl::swapBuffers()
{
    [[this->_view openGLContext] flushBuffer];
}

void
GLContext::Impl::destroyContext()
{
}

} // namespace WindowLibrary
