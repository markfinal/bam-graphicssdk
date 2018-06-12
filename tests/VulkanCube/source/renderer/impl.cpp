#include "impl.h"
#include "exception.h"
#include "log.h"

#include <functional>

Renderer::Impl::Impl()
    :
    _instance(nullptr, nullptr),
    _logical_device(nullptr, nullptr)
{}

Renderer::Impl::~Impl() = default;

PFN_vkDestroyInstance Renderer::Impl::VkFunctionTable::_destroy_instance = nullptr;
PFN_vkDestroyDevice Renderer::Impl::VkFunctionTable::_destroy_device = nullptr;

void
Renderer::Impl::VkFunctionTable::get_instance_functions(
    ::VkInstance inInstance)
{
    _destroy_instance = GETIFN(inInstance, vkDestroyInstance);
    _destroy_device = GETIFN(inInstance, vkDestroyDevice);
}

void
Renderer::Impl::VkFunctionTable::destroy_instance_wrapper(
    ::VkInstance inInstance)
{
    _destroy_instance(inInstance, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_device_wrapper(
    ::VkDevice inDevice)
{
    _destroy_device(inDevice, nullptr);
}

void
Renderer::Impl::create_instance()
{
    ::VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Cube";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // query extensions
    auto query_extensions = GETFN(vkEnumerateInstanceExtensionProperties);
    uint32_t num_extensions;
    auto ext_query_res = query_extensions(
        nullptr,
        &num_extensions,
        nullptr
    );
    std::vector< ::VkExtensionProperties> extensions(num_extensions);
    ext_query_res = query_extensions(
        nullptr,
        &num_extensions,
        extensions.data()
    );
    for (const auto &ext : extensions)
    {
        Log().get() << "Extension: " << ext.extensionName << ", v" << ext.specVersion << std::endl;
    }

    ::VkInstanceCreateInfo createInfo;
    //::VkAllocationCallbacks allocCbs;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // required
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = 0;
    //memset(&allocCbs, 0, sizeof(allocCbs));
    ::VkInstance instance;
    auto createInstanceFn = GETFN(vkCreateInstance);
    auto createInstanceRes = createInstanceFn(&createInfo, nullptr, &instance);
    if (VK_SUCCESS != createInstanceRes)
    {
        throw Exception("Unable to create instance");
    }
    this->_function_table.get_instance_functions(instance);
    decltype(this->_instance) temp(instance, this->_function_table.destroy_instance_wrapper);
    this->_instance = std::move(temp);
}

void
Renderer::Impl::enumerate_physics_devices()
{
    // enumerate physical devices
    auto enumPhysDevicesFn = GETIFN(this->_instance.get(), vkEnumeratePhysicalDevices);
    uint32_t numPhysicalDevices = 0;

    // get number of physical devices
    auto enumPhysDevicesRes = enumPhysDevicesFn(this->_instance.get(), &numPhysicalDevices, nullptr);
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to count physical devices");
    }
    if (0 == numPhysicalDevices)
    {
        throw Exception("There are no physical devices available on this hardware");
    }
    this->_physical_devices.resize(numPhysicalDevices);
    enumPhysDevicesRes = enumPhysDevicesFn(this->_instance.get(), &numPhysicalDevices, this->_physical_devices.data());
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to enumerate physical devices");
    }

    auto getPhysDeviceFeaturesFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceFeatures);
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
    auto getPDeviceQueueFamilyPropsFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceQueueFamilyProperties);
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
    auto createDeviceFn = GETIFN(this->_instance.get(), vkCreateDevice);
    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queue_info;
    ::VkDevice device;
    auto createDeviceRes = createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &device);
    if (VK_SUCCESS != createDeviceRes)
    {
        throw Exception("Unable to find create logical device");
    }
    decltype(this->_logical_device) temp(device, this->_function_table.destroy_device_wrapper);
    this->_logical_device = std::move(temp);
}
