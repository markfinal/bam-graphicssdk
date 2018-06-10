#include "impl.h"
#include "exception.h"

void
Renderer::Impl::clean_up()
{
    // destroy the logical device
    auto destroyDeviceFn = GETIFN(this->_instance, vkDestroyDevice);
    destroyDeviceFn(this->_logical_device, nullptr);

    auto destroyInstanceFn = GETIFN(this->_instance, vkDestroyInstance);
    destroyInstanceFn(this->_instance, nullptr);
}

void
Renderer::Impl::create_instance()
{
    auto createInstanceFn = GETFN(vkCreateInstance);
    ::VkInstanceCreateInfo createInfo;
    //::VkAllocationCallbacks allocCbs;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // required
    //memset(&allocCbs, 0, sizeof(allocCbs));
    auto createInstanceRes = createInstanceFn(&createInfo, nullptr, &this->_instance);
    if (VK_SUCCESS != createInstanceRes)
    {
        throw Exception("Unable to create instance");
    }
}

void
Renderer::Impl::enumerate_physics_devices()
{
    // enumerate physical devices
    auto enumPhysDevicesFn = GETIFN(this->_instance, vkEnumeratePhysicalDevices);
    uint32_t numPhysicalDevices = 0;

    // get number of physical devices
    auto enumPhysDevicesRes = enumPhysDevicesFn(this->_instance, &numPhysicalDevices, nullptr);
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to count physics devices");
    }
    this->_physical_devices.resize(numPhysicalDevices);
    enumPhysDevicesRes = enumPhysDevicesFn(this->_instance, &numPhysicalDevices, this->_physical_devices.data());
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to enumerate physics devices");
    }

    auto getPhysDeviceFeaturesFn = GETIFN(this->_instance, vkGetPhysicalDeviceFeatures);
    for (auto i = 0; i < numPhysicalDevices; ++i)
    {
        auto device = this->_physical_devices[i];
        VkPhysicalDeviceFeatures features;
        getPhysDeviceFeaturesFn(device, &features);
    }

    // arbitrary choice
    this->_physical_device_index = 0;
}

void
Renderer::Impl::create_logical_device()
{
    auto pDevice = this->_physical_devices[this->_physical_device_index];

    // query the family of queues available
    auto getPDeviceQueueFamilyPropsFn = GETIFN(this->_instance, vkGetPhysicalDeviceQueueFamilyProperties);
    uint32_t numQueueFamilyProperties = 0;
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilyProperties);
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, queueFamilyProperties.data());
    // assume that the first queue family is capable of graphics
    if (0 == (queueFamilyProperties[0].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
        throw Exception("Unable to find queue family with graphics support");
    }

    // logical devices need a queue
    VkDeviceQueueCreateInfo queue_info;
    memset(&queue_info, 0, sizeof(queue_info));
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;

    // create a logical device
    auto createDeviceFn = GETIFN(this->_instance, vkCreateDevice);
    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queue_info;
    auto createDeviceRes = createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &this->_logical_device);
    if (VK_SUCCESS != createDeviceRes)
    {
        throw Exception("Unable to find create logical device");
    }
}
