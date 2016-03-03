#include "vulkan/vulkan.h"
#include <Windows.h>

// these macros avoid repetition between stating the name of the function and the PFN_* type
#define GETPFN(_name) PFN_##_name
#define GETFN(_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(nullptr, #_name))
#define GETIFN(_instance,_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(_instance, #_name))

int CALLBACK
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    auto createInstanceFn = GETFN(vkCreateInstance);
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
    auto enumPhysDevicesFn = GETIFN(instance, vkEnumeratePhysicalDevices);
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

    auto getPhysDeviceFeaturesFn = GETIFN(instance, vkGetPhysicalDeviceFeatures);
    for (auto i = 0; i < numPhysicalDevices; ++i)
    {
        auto device = physicalDevices[i];
        VkPhysicalDeviceFeatures features;
        getPhysDeviceFeaturesFn(device, &features);
    }

    const auto pDevice = physicalDevices[0];

    // query the family of queues available
    auto getPDeviceQueueFamilyPropsFn = GETIFN(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    uint32_t numQueueFamilyProperties = 0;
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, nullptr);
    auto queueFamilyProperties = new VkQueueFamilyProperties[numQueueFamilyProperties];
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, queueFamilyProperties);
    // assume that the first queue family is capable of graphics
    if (0 == (queueFamilyProperties[0].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
        return -1;
    }

    // logical devices need a queue
    VkDeviceQueueCreateInfo queue_info;
    memset(&queue_info, 0, sizeof(queue_info));
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;

    // create a logical device
    auto createDeviceFn = GETIFN(instance, vkCreateDevice);
    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queue_info;
    VkDevice device;
    auto createDeviceRes = createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &device);
    if (VK_SUCCESS != createDeviceRes)
    {
        return -1;
    }

    // destroy the logical device
    auto destroyDeviceFn = GETIFN(instance, vkDestroyDevice);
    destroyDeviceFn(device, nullptr);

    delete[] queueFamilyProperties;
    delete[] physicalDevices;

    auto destroyInstanceFn = GETIFN(instance, vkDestroyInstance);
    destroyInstanceFn(instance, nullptr);
    return 0;
}
