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
    auto createInstanceRes = createInstanceFn(&createInfo, nullptr, &instance);
    if (VK_SUCCESS != createInstanceRes)
    {
        return -1;
    }

    // enumerate physical devices
    auto enumPhysDevicesFn = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
    uint32_t numPhysicalDevices = 0;

    // get number of physical devices
    auto enumPhysDevicesRes = enumPhysDevicesFn(instance, &numPhysicalDevices, nullptr);
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        // TODO: need some RAII on the instance
        return -1;
    }
    auto physicalDevices = new VkPhysicalDevice[numPhysicalDevices];
    enumPhysDevicesRes = enumPhysDevicesFn(instance, &numPhysicalDevices, physicalDevices);
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        // TODO: need some RAII on the instance
        return -1;
    }

    auto getPhysDeviceFeaturesFn = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures"));
    for (auto i = 0; i < numPhysicalDevices; ++i)
    {
        auto device = physicalDevices[i];
        VkPhysicalDeviceFeatures features;
        getPhysDeviceFeaturesFn(device, &features);
    }

    // create a logical device
    auto createDeviceFn = reinterpret_cast<PFN_vkCreateDevice>(vkGetInstanceProcAddr(instance, "vkCreateDevice"));
    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    VkDevice device;
    // TODO: going by the simpler examples, this needs at least one queue
    // perhaps that is why the nvidia driver crashes here as-is
    auto createDeviceRes = createDeviceFn(physicalDevices[0], &deviceCreateInfo, nullptr, &device);
    if (VK_SUCCESS != createDeviceRes)
    {
        return -1;
    }

    // destroy the logical device
    auto destroyDeviceFn = reinterpret_cast<PFN_vkDestroyDevice>(vkGetInstanceProcAddr(instance, "vkDestroyDevice"));
    destroyDeviceFn(device, nullptr);

    delete[] physicalDevices;

    auto destroyInstanceFn = reinterpret_cast<PFN_vkDestroyInstance>(vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
    destroyInstanceFn(instance, nullptr);
    return 0;
}
