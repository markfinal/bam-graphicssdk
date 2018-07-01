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
#include "windowlibrary/graphicswindow.h"
#include "windowlibrary/exception.h"

#if defined(D_BAM_PLATFORM_WINDOWS)
#include "platform/win32winlibimpl.h"
#elif defined(D_BAM_PLATFORM_LINUX)
#include "platform/linuxwinlibimpl.h"
#elif defined(D_BAM_PLATFORM_OSX)
#include "platform/macoswinlibimpl.h"
#else
#error Unsupported platform
#endif

namespace WindowLibrary
{

GraphicsWindow::GraphicsWindow()
    :
    _impl(new Impl(this))
{}

GraphicsWindow::~GraphicsWindow() = default;

uint32_t
GraphicsWindow::width() const
{
    auto impl = this->_impl.get();
    return impl->_width;
}

uint32_t
GraphicsWindow::height() const
{
    auto impl = this->_impl.get();
    return impl->_height;
}

void
GraphicsWindow::onCreate()
{}

void
GraphicsWindow::onDestroy()
{}

void
GraphicsWindow::onClose()
{}

} // namespace WindowLibrary
