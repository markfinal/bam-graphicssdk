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
    //::VkAllocationCallbacks allocCbs;
    ::VkInstance instance;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // required
    //memset(&allocCbs, 0, sizeof(allocCbs));
    auto result = createInstanceFn(&createInfo, nullptr, &instance);
    if (VK_SUCCESS == result)
    {
        auto destroyInstanceFn = reinterpret_cast<PFN_vkDestroyInstance>(vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
        destroyInstanceFn(instance, nullptr);
    }
    return 0;
}
