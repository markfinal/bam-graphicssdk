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
#include "renderer/renderer.h"
#include "log.h"
#include "appwindow.h"

#ifdef D_BAM_PLATFORM_WINDOWS
#include <Windows.h>
#else
#endif

#include <memory>

namespace
{

int
event_loop(
    Renderer *inRenderer)
{
#if defined(D_BAM_PLATFORM_WINDOWS)
    ::MSG msg;

    // loop until WM_QUIT(0) received
    while (::GetMessage(&msg, 0, 0, 0) > 0)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);

        inRenderer->draw_frame();
    }

    return static_cast<int>(msg.wParam);
#endif
}

} // anonymous namespace

#ifdef D_BAM_PLATFORM_WINDOWS
int CALLBACK
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
#else
int
main()
#endif
{
    Log().get() << "Vulkan cube test starting up..." << std::endl;
    try
    {
        std::unique_ptr<AppWindow> window(new AppWindow);
#ifdef D_BAM_PLATFORM_WINDOWS
        (void)nCmdShow;
        (void)lpCmdLine;
        (void)hPrevInstance;
        window->win32SetInstanceHandle(hInstance);
#endif
        window->init(256, 256, "Vulkan Cube");
        std::unique_ptr<Renderer> renderer(new Renderer(window.get()));
        renderer->init();

        window->show();

        Log().get() << "Vulkan cube test finished successfully" << std::endl;
        return event_loop(renderer.get());
    }
    catch (const std::exception &inEx)
    {
        Log().get() << "ERROR: " << inEx.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        Log().get() << "ERROR: Unhandled exception" << std::endl;
        return -2;
    }
}
