#include "renderer/renderer.h"
#include "log.h"

#ifdef D_BAM_PLATFORM_WINDOWS
#include <Windows.h>
#else
#endif

#include <memory>

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
        std::unique_ptr<Renderer> renderer(new Renderer);
        renderer->init();
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
    Log().get() << "Vulkan cube test finished successfully" << std::endl;
    return 0;
}
