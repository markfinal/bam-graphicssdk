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
#include "application.h"
#include "renderer.h"
#include "errorhandler.h"
#include "common.h"
#include "appwindow.h"

#include <string>

Application *Application::spInstance = 0;
Application *Application::GetInstance()
{
    return spInstance;
}

Application::Application(int UNUSEDARG(argc), char *UNUSEDARG(argv)[])
    :
    mpRenderer(nullptr),
    mpWindow(new AppWindow),
    mhWin32Instance(0),
    mi32ExitCode(0)
{
    if (0 != spInstance)
    {
        REPORTERROR("There is already an instance of the application running");
        return;
    }

    spInstance = this;
}

Application::~Application() = default;

void Application::SetWin32Instance(void *instance)
{
    mhWin32Instance = instance;
}

int Application::Run()
{
#ifdef D_BAM_PLATFORM_WINDOWS
    this->mpWindow->win32SetInstanceHandle(static_cast<::HINSTANCE>(mhWin32Instance));
#endif
    this->mpWindow->init();
    this->MainLoop();
    return this->mi32ExitCode;
}

void Application::MainLoop()
{
#if defined(D_BAM_PLATFORM_WINDOWS)
    ::MSG msg;

    // loop until WM_QUIT(0) received
    while(::GetMessage(&msg, 0, 0, 0) > 0)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    this->mi32ExitCode = (int)msg.wParam;
#elif defined(D_BAM_PLATFORM_LINUX)
    auto display = this->mpWindow->linuxDisplay();
    while (true)
    {
        while (::XPending(display) > 0)
        {
            ::XEvent event;
            ::XNextEvent(display, &event);
            switch (event.type)
            {
            case ClientMessage:
                if (event.xclient.data.l[0] == static_cast<int long>(this->mpWindow->linuxDeleteWindowMessage()))
                {
                    this->mpWindow->onDestroy();
                    this->mi32ExitCode = 0;
                    return;
                }
                break;
            }
        }
    }
#elif defined(D_BAM_PLATFORM_OSX)
    // TODO: possibly in an objective-C file
#else
#error Unsupported platform
#endif
}

void Application::SetRenderer(Renderer *renderer)
{
    this->mpRenderer = renderer;
}

Renderer *Application::GetRenderer()
{
    return this->mpRenderer;
}
