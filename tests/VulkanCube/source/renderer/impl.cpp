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
#include "impl.h"
#include "exception.h"
#include "log.h"

#include "../appwindow.h"

#if defined(D_BAM_PLATFORM_OSX)
#include "vulkan/vulkan_macos.h"
#endif

#include <algorithm>
#include <array>
#include <functional>

Renderer::Impl::Impl(
    AppWindow *inWindow)
    :
    _instance(nullptr, nullptr),
    _debug_callback(nullptr, nullptr),
    _window(inWindow),
    _surface(nullptr, nullptr),
    _logical_device(nullptr, nullptr),
    _swapchain(nullptr, nullptr)
{}

Renderer::Impl::~Impl() = default;

PFN_vkDestroyInstance Renderer::Impl::VkFunctionTable::_destroy_instance = nullptr;
PFN_vkDestroyDebugReportCallbackEXT Renderer::Impl::VkFunctionTable::_destroy_debug_callback = nullptr;
std::function<void(::VkDebugReportCallbackEXT, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_debug_callback_boundinstance;
PFN_vkDestroyDevice Renderer::Impl::VkFunctionTable::_destroy_device = nullptr;
PFN_vkDestroySurfaceKHR Renderer::Impl::VkFunctionTable::_destroy_surface_khr = nullptr;
std::function<void(::VkSurfaceKHR, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_surface_khr_boundinstance;
PFN_vkDestroySwapchainKHR Renderer::Impl::VkFunctionTable::_destroy_swapchain_khr = nullptr;
std::function<void(::VkSwapchainKHR, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_swapchain_khr_bounddevice;

void
Renderer::Impl::VkFunctionTable::get_instance_functions(
    ::VkInstance inInstance)
{
    _destroy_instance = GETIFN(inInstance, vkDestroyInstance);
    _destroy_debug_callback = GETIFN(inInstance, vkDestroyDebugReportCallbackEXT);
    _destroy_debug_callback_boundinstance = std::bind(_destroy_debug_callback, inInstance, std::placeholders::_1, std::placeholders::_2);
    _destroy_device = GETIFN(inInstance, vkDestroyDevice);
    _destroy_surface_khr = GETIFN(inInstance, vkDestroySurfaceKHR);
    _destroy_surface_khr_boundinstance = std::bind(_destroy_surface_khr, inInstance, std::placeholders::_1, std::placeholders::_2);
}

void
Renderer::Impl::VkFunctionTable::get_device_functions(
    ::VkInstance inInstance,
    ::VkDevice inDevice)
{
    _destroy_swapchain_khr = GETIFN(inInstance, vkDestroySwapchainKHR);
    _destroy_swapchain_khr_bounddevice = std::bind(_destroy_swapchain_khr, inDevice, std::placeholders::_1, std::placeholders::_2);
}

void
Renderer::Impl::VkFunctionTable::destroy_instance_wrapper(
    ::VkInstance inInstance)
{
    Log().get() << "Destroying VkInstance 0x" << std::hex << inInstance << std::endl;
    _destroy_instance(inInstance, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_debug_callback_wrapper(
    ::VkDebugReportCallbackEXT inDebugCallback)
{
    Log().get() << "Destroying VkDebugReportCallbackEXT 0x" << std::hex << inDebugCallback << std::endl;
    _destroy_debug_callback_boundinstance(inDebugCallback, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_device_wrapper(
    ::VkDevice inDevice)
{
    Log().get() << "Destroying VkDevice 0x" << std::hex << inDevice << std::endl;
    _destroy_device(inDevice, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_surface_khr_wrapper(
    ::VkSurfaceKHR inSurface)
{
    Log().get() << "Destroying VkSurfaceKHR 0x" << std::hex << inSurface << std::endl;
    _destroy_surface_khr_boundinstance(inSurface, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_swapchain_khr_wrapper(
    ::VkSwapchainKHR inSwapchain)
{
    Log().get() << "Destroying VkSwapchainKHR 0x" << std::hex << inSwapchain << std::endl;
    _destroy_swapchain_khr_bounddevice(inSwapchain, nullptr);
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
    Log().get() << "Instance extensions:" << std::endl;
    for (const auto &ext : extensions)
    {
        Log().get() << "\t" << ext.extensionName << ", v" << ext.specVersion << std::endl;
    }

    // query layers
    auto query_layers_fn = GETFN(vkEnumerateInstanceLayerProperties);
    uint32_t num_layers;
    auto layer_query_res = query_layers_fn(
        &num_layers,
        nullptr
    );
    std::vector<::VkLayerProperties> layers(num_layers);
    layer_query_res = query_layers_fn(
        &num_layers,
        layers.data()
    );
    Log().get() << "Instance layers:" << std::endl;
    for (const auto &layer : layers)
    {
        Log().get() << "\t" << layer.layerName << ", " << layer.description << ", " << layer.implementationVersion << ", " << layer.specVersion << std::endl;
    }

    // now look for extensions we want to use
    std::vector<const char *> instanceExtensionNames;
    {
        auto khr_surface_it = std::find_if(extensions.begin(), extensions.end(), [](::VkExtensionProperties &extension)
        {
            return (0 == strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME));
        });
        if (khr_surface_it == extensions.end())
        {
            throw Exception("Instance does not support the " VK_KHR_SURFACE_EXTENSION_NAME " extension");
        }
        instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }
    {
        auto it = std::find_if(extensions.begin(), extensions.end(), [](::VkExtensionProperties &extension)
        {
            return (0 == strcmp(extension.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        });
        if (it == extensions.end())
        {
            Log().get() << "Instance does not support the " VK_EXT_DEBUG_REPORT_EXTENSION_NAME " extension" << std::endl;
        }
        else
        {
            instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
    }
#if defined(D_BAM_PLATFORM_WINDOWS)
    {
        auto khr_win32_surface_it = std::find_if(extensions.begin(), extensions.end(), [](::VkExtensionProperties &extension)
        {
            return (0 == strcmp(extension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
        });
        if (khr_win32_surface_it == extensions.end())
        {
            throw Exception("Instance does not support the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME " extension");
        }
        instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#elif defined(D_BAM_PLATFORM_OSX)
    {
        auto mvk_macos_surface_it = std::find_if(extensions.begin(), extensions.end(), [](::VkExtensionProperties &extension)
        {
            return (0 == strcmp(extension.extensionName, VK_MVK_MACOS_SURFACE_EXTENSION_NAME));
        });
        if (mvk_macos_surface_it == extensions.end())
        {
            throw Exception("Instance does not support the " VK_MVK_MACOS_SURFACE_EXTENSION_NAME " extension");
        }
        instanceExtensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
    }
#else
#error Unsupported platform
#endif

    // now look for layers we want to use
    std::vector<const char *> instanceLayerNames;
#if defined(D_BAM_PLATFORM_OSX)
    {
        auto it = std::find_if(layers.begin(), layers.end(), [](::VkLayerProperties &layer)
        {
            return (0 == strcmp(layer.layerName, "MoltenVK"));
        });
        if (it == layers.end())
        {
            Log().get() << "Instance does not support the " "MoltenVK" " layer" << std::endl;
        }
        else
        {
            instanceLayerNames.push_back("MoltenVK");
        }
    }
#else
    {
        auto it = std::find_if(layers.begin(), layers.end(), [](::VkLayerProperties &layer)
        {
            return (0 == strcmp(layer.layerName, "VK_LAYER_LUNARG_standard_validation"));
        });
        if (it == layers.end())
        {
            Log().get() << "Instance does not support the " "VK_LAYER_LUNARG_standard_validation" " layer" << std::endl;
        }
        else
        {
            instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
#endif

    ::VkInstanceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // required
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayerNames.size());
    createInfo.ppEnabledLayerNames = instanceLayerNames.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionNames.size());
    createInfo.ppEnabledExtensionNames = instanceExtensionNames.data();
    //::VkAllocationCallbacks allocCbs;
    //memset(&allocCbs, 0, sizeof(allocCbs));
    ::VkInstance instance;
    auto createInstanceFn = GETFN(vkCreateInstance);
    auto createInstanceRes = createInstanceFn(&createInfo, nullptr, &instance);
    if (VK_SUCCESS != createInstanceRes)
    {
        throw Exception("Unable to create instance");
    }
    this->_function_table.get_instance_functions(instance);
    this->_instance = { instance, this->_function_table.destroy_instance_wrapper };
}

void
Renderer::Impl::init_debug_callback()
{
    auto fn = GETIFN(this->_instance.get(), vkCreateDebugReportCallbackEXT);
    if (nullptr == fn)
    {
        return;
    }

    ::VkDebugReportCallbackCreateInfoEXT createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    createInfo.flags =
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT ||
        VK_DEBUG_REPORT_WARNING_BIT_EXT ||
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ||
        VK_DEBUG_REPORT_ERROR_BIT_EXT ||
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    createInfo.pfnCallback = debug_callback;
    ::VkDebugReportCallbackEXT callback;
    fn(
        this->_instance.get(),
        &createInfo,
        nullptr,
        &callback
    );
    this->_debug_callback = { callback, this->_function_table.destroy_debug_callback_wrapper };
}

VkBool32
Renderer::Impl::debug_callback(
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage,
    void*                                       pUserData)
{
    (void)flags;
    (void)objectType;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    Log().get() << pLayerPrefix << ": " << pMessage << std::endl;
    return VK_FALSE;
}

void
Renderer::Impl::create_window_surface()
{
#if defined(D_BAM_PLATFORM_WINDOWS)
    ::VkWin32SurfaceCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = this->_window->getNativeWindowHandle();
    createInfo.hinstance = ::GetModuleHandle(nullptr);

    auto createWindowSurfaceFn = GETIFN(this->_instance.get(), vkCreateWin32SurfaceKHR);
    ::VkSurfaceKHR surface;
    auto createWindowSurfaceRes = createWindowSurfaceFn(
        this->_instance.get(),
        &createInfo,
        nullptr,
        &surface
    );
    if (VK_SUCCESS != createWindowSurfaceRes)
    {
        throw Exception("Unable to create window surface");
    }

    this->_surface = { surface, this->_function_table.destroy_surface_khr_wrapper };
#elif defined(D_BAM_PLATFORM_OSX)
    ::VkMacOSSurfaceCreateInfoMVK createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = this->_window->macosGetViewHandle();

    auto createWindowSurfaceFn = GETIFN(this->_instance.get(), vkCreateMacOSSurfaceMVK);
    ::VkSurfaceKHR surface;
    auto createWindowSurfaceRes = createWindowSurfaceFn(
        this->_instance.get(),
        &createInfo,
        nullptr,
        &surface
    );
    if (VK_SUCCESS != createWindowSurfaceRes)
    {
        throw Exception("Unable to create window surface");
    }

    this->_surface = { surface, this->_function_table.destroy_surface_khr_wrapper };
#else
#error Unsupported platform
#endif
}

void
Renderer::Impl::enumerate_physical_devices()
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
    Log().get() << "Found " << numPhysicalDevices << " physical devices" << std::endl;
    this->_physical_devices.resize(numPhysicalDevices);
    enumPhysDevicesRes = enumPhysDevicesFn(this->_instance.get(), &numPhysicalDevices, this->_physical_devices.data());
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to enumerate physical devices");
    }

    // enumerate physical device properties
    auto enumeratePhysicalDevicePropsFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceProperties);
    for (auto i = 0u; i < numPhysicalDevices; ++i)
    {
        auto device = this->_physical_devices[i];
        ::VkPhysicalDeviceProperties props;
        enumeratePhysicalDevicePropsFn(device, &props);
        Log().get() << "Properties of physical device " << i << std::endl;
        Log().get() << "\tAPI version : " << props.apiVersion << std::endl;
        Log().get() << "\tDevice ID : " << props.deviceID << std::endl;
        Log().get() << "\tDevice name : " << props.deviceName << std::endl;
        Log().get() << "\tDevice type : " << props.deviceType << std::endl;
        Log().get() << "\tDriver version : " << props.driverVersion << std::endl;
        Log().get() << "\tPipeline Cache UUID : " << props.pipelineCacheUUID << std::endl;
        Log().get() << "\tLimits..." << std::endl;
        Log().get() << "\tSparse properties..." << std::endl;
    }

    // enumerate physical device features
    auto getPhysDeviceFeaturesFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceFeatures);
    for (auto i = 0u; i < numPhysicalDevices; ++i)
    {
        auto device = this->_physical_devices[i];
        VkPhysicalDeviceFeatures features;
        getPhysDeviceFeaturesFn(device, &features);
        Log().get() << "Features of physical device " << i << std::endl;

#define LOG_FEATURE(_feature) Log().get() << "\t"#_feature << ": " << features._feature << std::endl

        LOG_FEATURE(alphaToOne);
        LOG_FEATURE(depthBiasClamp);
        LOG_FEATURE(depthBounds);
        LOG_FEATURE(depthClamp);
        LOG_FEATURE(drawIndirectFirstInstance);
        LOG_FEATURE(dualSrcBlend);
        LOG_FEATURE(fillModeNonSolid);
        LOG_FEATURE(fragmentStoresAndAtomics);
        LOG_FEATURE(fullDrawIndexUint32);
        LOG_FEATURE(geometryShader);
        LOG_FEATURE(imageCubeArray);
        LOG_FEATURE(independentBlend);
        LOG_FEATURE(inheritedQueries);
        LOG_FEATURE(largePoints);
        LOG_FEATURE(logicOp);
        LOG_FEATURE(multiDrawIndirect);
        LOG_FEATURE(multiViewport);
        LOG_FEATURE(occlusionQueryPrecise);
        LOG_FEATURE(pipelineStatisticsQuery);
        LOG_FEATURE(robustBufferAccess);
        LOG_FEATURE(samplerAnisotropy);
        LOG_FEATURE(sampleRateShading);
        LOG_FEATURE(shaderClipDistance);
        LOG_FEATURE(shaderCullDistance);
        LOG_FEATURE(shaderFloat64);
        LOG_FEATURE(shaderImageGatherExtended);
        LOG_FEATURE(shaderInt16);
        LOG_FEATURE(shaderInt64);
        LOG_FEATURE(shaderResourceMinLod);
        LOG_FEATURE(shaderResourceResidency);
        LOG_FEATURE(shaderSampledImageArrayDynamicIndexing);
        LOG_FEATURE(shaderStorageBufferArrayDynamicIndexing);
        LOG_FEATURE(shaderStorageImageArrayDynamicIndexing);
        LOG_FEATURE(shaderStorageImageExtendedFormats);
        LOG_FEATURE(shaderStorageImageMultisample);
        LOG_FEATURE(shaderStorageImageReadWithoutFormat);
        LOG_FEATURE(shaderStorageImageWriteWithoutFormat);
        LOG_FEATURE(shaderTessellationAndGeometryPointSize);
        LOG_FEATURE(shaderUniformBufferArrayDynamicIndexing);
        LOG_FEATURE(sparseBinding);
        LOG_FEATURE(sparseResidency16Samples);
        LOG_FEATURE(sparseResidency2Samples);
        LOG_FEATURE(sparseResidency4Samples);
        LOG_FEATURE(sparseResidency8Samples);
        LOG_FEATURE(sparseResidencyAliased);
        LOG_FEATURE(sparseResidencyBuffer);
        LOG_FEATURE(sparseResidencyImage2D);
        LOG_FEATURE(sparseResidencyImage3D);
        LOG_FEATURE(tessellationShader);
        LOG_FEATURE(textureCompressionASTC_LDR);
        LOG_FEATURE(textureCompressionBC);
        LOG_FEATURE(textureCompressionETC2);
        LOG_FEATURE(variableMultisampleRate);
        LOG_FEATURE(vertexPipelineStoresAndAtomics);

#undef LOG_FEATURE
    }

    // arbitrary choice
    this->_physical_device_index = 0;
}

void
Renderer::Impl::create_logical_device()
{
    auto pDevice = this->_physical_devices[this->_physical_device_index];

    // enumerate physical device extensions
    auto enumDeviceExtensionPropertiesFn = GETIFN(this->_instance.get(), vkEnumerateDeviceExtensionProperties);
    uint32_t numDeviceExtensions = 0;
    enumDeviceExtensionPropertiesFn(pDevice, nullptr, &numDeviceExtensions, nullptr);
    Log().get() << "Found " << numDeviceExtensions << " device extensions" << std::endl;
    std::vector<::VkExtensionProperties> deviceExtensions(numDeviceExtensions);
    enumDeviceExtensionPropertiesFn(pDevice, nullptr, &numDeviceExtensions, deviceExtensions.data());
    for (auto i = 0u; i < numDeviceExtensions; ++i)
    {
        const auto &ext = deviceExtensions[i];
        Log().get() << "\t" << i << ": " << ext.extensionName << ", " << ext.specVersion << std::endl;
    }

    // enumerate physical device layers
    auto enumDeviceLayerPropertiesFn = GETIFN(this->_instance.get(), vkEnumerateDeviceLayerProperties);
    uint32_t numDeviceLayers = 0;
    enumDeviceLayerPropertiesFn(pDevice, &numDeviceLayers, nullptr);
    Log().get() << "Found " << numDeviceLayers << " device layers" << std::endl;
    std::vector<::VkLayerProperties> deviceLayers(numDeviceLayers);
    enumDeviceLayerPropertiesFn(pDevice, &numDeviceLayers, deviceLayers.data());
    for (auto i = 0u; i < numDeviceLayers; ++i)
    {
        const auto &layer = deviceLayers[i];
        Log().get() << "\t" << i << ": " << layer.layerName << ", " << layer.description << ", " << layer.implementationVersion << ", " << layer.specVersion << std::endl;
    }

    // query the family of queues available
    auto getPDeviceQueueFamilyPropsFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceQueueFamilyProperties);
    uint32_t numQueueFamilyProperties = 0;
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, nullptr);
    if (0 == numQueueFamilyProperties)
    {
        throw Exception("Unable to find any queue families on this physical device");
    }
    Log().get() << "Found " << numQueueFamilyProperties << " queue families on this physical device" << std::endl;

    // assume that the first queue family is capable of graphics
    auto graphics_family_queue_index = 0;
    // assume that the same queue family is capable of presentation
    auto present_family_queue_index = 0;

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilyProperties);
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, queueFamilyProperties.data());
    if (0 == (queueFamilyProperties[graphics_family_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
        throw Exception("Unable to find queue family with graphics support on this physical device");
    }

    ::VkBool32 presentSupport = false;
    auto getPDeviceSurfaceSupportFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceSurfaceSupportKHR);
    getPDeviceSurfaceSupportFn(
        pDevice,
        present_family_queue_index,
        this->_surface.get(),
        &presentSupport
    );
    if (!presentSupport)
    {
        throw Exception("Physical device cannot support presentation on the window surface");
    }

    std::vector<const char *> deviceExtensionsRequired;
    auto swap_chain_ext_it = std::find_if(deviceExtensions.begin(), deviceExtensions.end(), [](const ::VkExtensionProperties &extProp)
    {
        return (0 == strcmp(extProp.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME));
    });
    if (swap_chain_ext_it == deviceExtensions.end())
    {
        throw Exception("Device does not support extension " VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    Log().get() << "Device extension " << VK_KHR_SWAPCHAIN_EXTENSION_NAME << " requested" << std::endl;
    deviceExtensionsRequired.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    std::vector<const char *> deviceLayersRequired;
#if defined(D_BAM_PLATFORM_OSX)
    {
        auto it = std::find_if(deviceLayers.begin(), deviceLayers.end(), [](::VkLayerProperties &layer)
        {
            return (0 == strcmp(layer.layerName, "MoltenVK"));
        });
        if (it == deviceLayers.end())
        {
            Log().get() << "Instance does not support the " "MoltenVK" " layer" << std::endl;
        }
        else
        {
            deviceLayersRequired.push_back("MoltenVK");
        }
    }
#else
    {
        auto it = std::find_if(deviceLayers.begin(), deviceLayers.end(), [](::VkLayerProperties &layer)
        {
            return (0 == strcmp(layer.layerName, "VK_LAYER_LUNARG_standard_validation"));
        });
        if (it == deviceLayers.end())
        {
            Log().get() << "Instance does not support the " "VK_LAYER_LUNARG_standard_validation" " layer" << std::endl;
        }
        else
        {
            deviceLayersRequired.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
#endif

    // logical devices need a queue
    VkDeviceQueueCreateInfo queue_info;
    memset(&queue_info, 0, sizeof(queue_info));
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = graphics_family_queue_index;
    queue_info.queueCount = 1;
    float queuePriority = 1.0f;
    queue_info.pQueuePriorities = &queuePriority; // Note: this is essential for at least MoltenVK, which does not check whether this is null or not

    // create a logical device
    auto createDeviceFn = GETIFN(this->_instance.get(), vkCreateDevice);
    VkDeviceCreateInfo deviceCreateInfo;
    memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queue_info;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsRequired.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsRequired.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(deviceLayersRequired.size());
    deviceCreateInfo.ppEnabledLayerNames = deviceLayersRequired.data();
    ::VkDevice device;
    auto createDeviceRes = createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &device);
    if (VK_SUCCESS != createDeviceRes)
    {
        throw Exception("Unable to find create logical device");
    }
    this->_logical_device = { device, this->_function_table.destroy_device_wrapper };
    this->_function_table.get_device_functions(this->_instance.get(), this->_logical_device.get());

    auto getQueueFn = GETIFN(this->_instance.get(), vkGetDeviceQueue);
    auto graphics_queue_index = 0;
    getQueueFn(
        this->_logical_device.get(),
        graphics_family_queue_index,
        graphics_queue_index,
        &this->_graphics_queue
    );
    auto present_queue_index = 0;
    getQueueFn(
        this->_logical_device.get(),
        present_family_queue_index,
        present_queue_index,
        &this->_present_queue
    );
}

void
Renderer::Impl::create_swapchain()
{
    auto pDevice = this->_physical_devices[this->_physical_device_index];

    auto getSurfaceCapsFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    ::VkSurfaceCapabilitiesKHR surfaceCaps;
    getSurfaceCapsFn(
        pDevice,
        this->_surface.get(),
        &surfaceCaps
    );
    Log().get() << "Surface capabilities" << std::endl;
    Log().get() << "Image count: [" << surfaceCaps.minImageCount << ", " << surfaceCaps.maxImageCount << "]" << std::endl;
    Log().get() << "Image extent: [(" << surfaceCaps.minImageExtent.width << "x" << surfaceCaps.minImageExtent.height << "), (" << surfaceCaps.maxImageExtent.width << "x" << surfaceCaps.maxImageExtent.height << ")]" << std::endl;

    uint32_t surfaceFormatCount = 0;
    auto getSurfaceFormatsFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceSurfaceFormatsKHR);
    getSurfaceFormatsFn(
        pDevice,
        this->_surface.get(),
        &surfaceFormatCount,
        nullptr
    );
    std::vector<::VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    getSurfaceFormatsFn(
        pDevice,
        this->_surface.get(),
        &surfaceFormatCount,
        surfaceFormats.data()
    );
    Log().get() << surfaceFormatCount << " surface formats" << std::endl;
    for (const auto format : surfaceFormats)
    {
        Log().get() << format.format << " " << format.colorSpace << std::endl;
    }

    uint32_t presentModeCount = 0;
    auto getPresentModesFn = GETIFN(this->_instance.get(), vkGetPhysicalDeviceSurfacePresentModesKHR);
    getPresentModesFn(
        pDevice,
        this->_surface.get(),
        &presentModeCount,
        nullptr
    );
    std::vector<::VkPresentModeKHR> presentModes(presentModeCount);
    getPresentModesFn(
        pDevice,
        this->_surface.get(),
        &presentModeCount,
        presentModes.data()
    );
    Log().get() << presentModeCount << " present modes" << std::endl;
    for (const auto pMode : presentModes)
    {
        Log().get() << pMode << std::endl;
    }

    ::VkSwapchainCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->_surface.get();
    createInfo.minImageCount = surfaceCaps.minImageCount;
    createInfo.imageFormat = surfaceFormats[0].format;
    createInfo.imageColorSpace = surfaceFormats[0].colorSpace;
    createInfo.imageExtent = surfaceCaps.maxImageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentModes[0];
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    ::VkSwapchainKHR swapchain;
    auto createSwapchainFn = GETIFN(this->_instance.get(), vkCreateSwapchainKHR);
    auto createSwapchainRes = createSwapchainFn(
        this->_logical_device.get(),
        &createInfo,
        nullptr,
        &swapchain
    );
    if (VK_SUCCESS != createSwapchainRes)
    {
        throw Exception("Unable to create swapchain");
    }
    this->_swapchain = { swapchain, this->_function_table.destroy_swapchain_khr_wrapper };

    auto getswapchainimagesFn = GETIFN(this->_instance.get(), vkGetSwapchainImagesKHR);
    uint32_t swapchain_imagecount = 0;
    getswapchainimagesFn(
        this->_logical_device.get(),
        this->_swapchain.get(),
        &swapchain_imagecount,
        nullptr
    );
    this->_swapchain_images.resize(swapchain_imagecount);
    getswapchainimagesFn(
        this->_logical_device.get(),
        this->_swapchain.get(),
        &swapchain_imagecount,
        this->_swapchain_images.data()
    );
}
