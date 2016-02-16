#include "vulkan/vulkan.h"
#include <Windows.h>

int CALLBACK
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    auto createInstanceFn = reinterpret_cast<PFN_vkCreateInstance>(vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
    ::VkInstanceCreateInfo createInfo;
    ::VkAllocationCallbacks allocCbs;
    ::VkInstance instance;
    memset(&createInfo, 0, sizeof(createInfo));
    memset(&allocCbs, 0, sizeof(allocCbs));
    auto result = createInstanceFn(&createInfo, &allocCbs, &instance);
    if (VK_SUCCESS == result)
    {
        auto destroyInstanceFn = reinterpret_cast<PFN_vkDestroyInstance>(vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
        destroyInstanceFn(instance, &allocCbs);
    }
    return 0;
}
