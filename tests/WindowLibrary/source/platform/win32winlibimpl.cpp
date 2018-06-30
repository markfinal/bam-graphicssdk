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
#include "windowlibrary/winlib.h"
#include "windowlibrary/exception.h"
#include "win32winlibimpl.h"

#include <cassert>

namespace
{

::LRESULT CALLBACK
WindowProcBootstrap(
    ::HWND hWnd,
    ::UINT Msg,
    ::WPARAM wParam,
    ::LPARAM lParam)
{
    WindowLibrary::GraphicsWindow *window = nullptr;
    if (WM_CREATE == Msg)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        window = static_cast<WindowLibrary::GraphicsWindow*>(lpcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
        window = reinterpret_cast<WindowLibrary::GraphicsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    return window->win32MessageProc(hWnd, Msg, wParam, lParam);
}

} // anonymous namespace

namespace WindowLibrary
{

GraphicsWindow::Impl::Impl(
    GraphicsWindow *inParent)
    :
    _parent(inParent),
    _instance(nullptr),
    _className("GraphicsWindowClass")
{}

GraphicsWindow::Impl::~Impl()
{
    this->destroyWindow();
    this->unregisterWindowClass();
    this->_parent = nullptr;
}

void
GraphicsWindow::Impl::registerWindowClass()
{
    assert(nullptr != this->_instance);
    ::WNDCLASSEX windowClass;
    ::ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.hInstance = this->_instance;
    windowClass.lpfnWndProc = WindowProcBootstrap;
    windowClass.lpszClassName = this->_className.c_str();
    UINT style = CS_OWNDC;
    windowClass.style = style;
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);

    ::ATOM returnValue = ::RegisterClassEx(&windowClass);
    if (0 == returnValue)
    {
        throw Win32FailedToRegisterClass();
    }
}

void
GraphicsWindow::Impl::unregisterWindowClass()
{
    assert(nullptr != this->_instance);
    ::BOOL result = ::UnregisterClass(this->_className.c_str(), this->_instance);
    if (0 == result)
    {
        throw Win32FailedToUnregisterClass();
    }
}

void
GraphicsWindow::Impl::createWindow(
    const uint32_t inWidth,
    const uint32_t inHeight)
{
    assert(nullptr != this->_instance);
    DWORD exStyle = 0;
    std::string mainWindowName("OpenGL triangle");
    DWORD style = WS_OVERLAPPEDWINDOW;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    ::HWND parentWindow = 0;
    ::HMENU menuHandle = 0;
    ::LPVOID lpParam = this->_parent;

    this->_windowHandle = ::CreateWindowEx(
        exStyle,
        this->_className.c_str(),
        mainWindowName.c_str(),
        style,
        x,
        y,
        inWidth,
        inHeight,
        parentWindow,
        menuHandle,
        this->_instance,
        lpParam);
    if (0 == this->_windowHandle)
    {
        throw Win32FailedToCreateWindow();
    }

    this->_parent->onCreate();
    this->_width = inWidth;
    this->_height = inHeight;

    // show window
    ::UpdateWindow(this->_windowHandle);
    ::ShowWindow(this->_windowHandle, SW_SHOWDEFAULT);
}

void
GraphicsWindow::Impl::destroyWindow()
{
    if (0 != this->_windowHandle && ::IsWindow(this->_windowHandle))
    {
        BOOL result = ::DestroyWindow(this->_windowHandle);
        if (0 == result)
        {
            throw Win32FailedToDestroyWindow();
        }
    }
}

} // namespace WindowLibrary
