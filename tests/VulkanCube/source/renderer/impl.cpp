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
    _swapchain(nullptr, nullptr),
    _renderPass(nullptr, nullptr),
    _commandPool(nullptr, nullptr)
{}

Renderer::Impl::~Impl() = default;

PFN_vkDestroyInstance Renderer::Impl::VkFunctionTable::_destroy_instance = nullptr;
PFN_vkDestroyDebugReportCallbackEXT Renderer::Impl::VkFunctionTable::_destroy_debug_callback = nullptr;
std::function<void(::VkDebugReportCallbackEXT, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_debug_callback_boundinstance;
PFN_vkDeviceWaitIdle Renderer::Impl::VkFunctionTable::_device_waitidle = nullptr;
PFN_vkDestroyDevice Renderer::Impl::VkFunctionTable::_destroy_device = nullptr;
PFN_vkDestroySwapchainKHR Renderer::Impl::VkFunctionTable::_destroy_swapchain_khr = nullptr;
std::function<void(::VkSwapchainKHR, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_swapchain_khr_bounddevice;
PFN_vkDestroyImageView Renderer::Impl::VkFunctionTable::_destroy_imageview = nullptr;
std::function<void(::VkImageView, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_imageview_bounddevice;
PFN_vkDestroyRenderPass Renderer::Impl::VkFunctionTable::_destroy_renderpass = nullptr;
std::function<void(::VkRenderPass, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_renderpass_bounddevice;
PFN_vkDestroyFramebuffer Renderer::Impl::VkFunctionTable::_destroy_framebuffer = nullptr;
std::function<void(::VkFramebuffer, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_framebuffer_bounddevice;
PFN_vkDestroyCommandPool Renderer::Impl::VkFunctionTable::_destroy_commandpool = nullptr;
std::function<void(::VkCommandPool, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_commandpool_bounddevice;
PFN_vkDestroySemaphore Renderer::Impl::VkFunctionTable::_destroy_semaphore = nullptr;
std::function<void(::VkSemaphore, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_semaphore_bounddevice;
PFN_vkDestroyFence Renderer::Impl::VkFunctionTable::_destroy_fence = nullptr;
std::function<void(::VkFence, const ::VkAllocationCallbacks*)> Renderer::Impl::VkFunctionTable::_destroy_fence_bounddevice;

void
Renderer::Impl::VkFunctionTable::get_instance_functions(
    ::VkInstance inInstance)
{
    _destroy_instance = GETIFN(inInstance, vkDestroyInstance);
    _destroy_debug_callback = GETIFN(inInstance, vkDestroyDebugReportCallbackEXT);
    _destroy_debug_callback_boundinstance = std::bind(_destroy_debug_callback, inInstance, std::placeholders::_1, std::placeholders::_2);
    _device_waitidle = GETIFN(inInstance, vkDeviceWaitIdle);
    _destroy_device = GETIFN(inInstance, vkDestroyDevice);
}

void
Renderer::Impl::VkFunctionTable::get_device_functions(
    ::VkDevice inDevice)
{
    _destroy_swapchain_khr = GETDFN(inDevice, vkDestroySwapchainKHR);
    _destroy_swapchain_khr_bounddevice = std::bind(_destroy_swapchain_khr, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_imageview = GETDFN(inDevice, vkDestroyImageView);
    _destroy_imageview_bounddevice = std::bind(_destroy_imageview, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_renderpass = GETDFN(inDevice, vkDestroyRenderPass);
    _destroy_renderpass_bounddevice = std::bind(_destroy_renderpass, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_framebuffer = GETDFN(inDevice, vkDestroyFramebuffer);
    _destroy_framebuffer_bounddevice = std::bind(_destroy_framebuffer, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_commandpool = GETDFN(inDevice, vkDestroyCommandPool);
    _destroy_commandpool_bounddevice = std::bind(_destroy_commandpool, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_semaphore = GETDFN(inDevice, vkDestroySemaphore);
    _destroy_semaphore_bounddevice = std::bind(_destroy_semaphore, inDevice, std::placeholders::_1, std::placeholders::_2);
    _destroy_fence = GETDFN(inDevice, vkDestroyFence);
    _destroy_fence_bounddevice = std::bind(_destroy_fence, inDevice, std::placeholders::_1, std::placeholders::_2);
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
    _device_waitidle(inDevice);
    _destroy_device(inDevice, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_swapchain_khr_wrapper(
    ::VkSwapchainKHR inSwapchain)
{
    Log().get() << "Destroying VkSwapchainKHR 0x" << std::hex << inSwapchain << std::endl;
    _destroy_swapchain_khr_bounddevice(inSwapchain, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_imageview_wrapper(
    ::VkImageView inImageView)
{
    Log().get() << "Destroying VkImageView 0x" << std::hex << inImageView << std::endl;
    _destroy_imageview_bounddevice(inImageView, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_renderpass_wrapper(
    ::VkRenderPass inRenderPass)
{
    Log().get() << "Destroying VkRenderPass 0x" << std::hex << inRenderPass << std::endl;
    _destroy_renderpass_bounddevice(inRenderPass, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_framebuffer_wrapper(
    ::VkFramebuffer inFrameBuffer)
{
    Log().get() << "Destroying VkFrameBuffer 0x" << std::hex << inFrameBuffer << std::endl;
    _destroy_framebuffer_bounddevice(inFrameBuffer, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_commandpool_wrapper(
    ::VkCommandPool inCommandPool)
{
    Log().get() << "Destroying VkCommandPool 0x" << std::hex << inCommandPool << std::endl;
    _destroy_commandpool_bounddevice(inCommandPool, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_semaphore_wrapper(
    ::VkSemaphore inSemaphore)
{
    Log().get() << "Destroying VkSemaphore 0x" << std::hex << inSemaphore << std::endl;
    _destroy_semaphore_bounddevice(inSemaphore, nullptr);
}

void
Renderer::Impl::VkFunctionTable::destroy_fence_wrapper(
    ::VkFence inFence)
{
    Log().get() << "Destroying VkFence 0x" << std::hex << inFence << std::endl;
    _destroy_fence_bounddevice(inFence, nullptr);
}

void
Renderer::Impl::create_instance()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
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
    VK_ERR_CHECK(query_extensions(
        nullptr,
        &num_extensions,
        nullptr
    ));
    std::vector< ::VkExtensionProperties> extensions(num_extensions);
    VK_ERR_CHECK(query_extensions(
        nullptr,
        &num_extensions,
        extensions.data()
    ));
    Log().get() << "Found the following " << extensions.size() << " INSTANCE extensions:" << std::endl;
    for (const auto &ext : extensions)
    {
        Log().get() << "\t" << ext.extensionName << ", v" << ext.specVersion << std::endl;
    }

    // query layers
    auto query_layers_fn = GETFN(vkEnumerateInstanceLayerProperties);
    uint32_t num_layers;
    VK_ERR_CHECK(query_layers_fn(
        &num_layers,
        nullptr
    ));
    std::vector<::VkLayerProperties> layers(num_layers);
    VK_ERR_CHECK(query_layers_fn(
        &num_layers,
        layers.data()
    ));
    Log().get() << "Found the following " << layers.size() << " INSTANCE layers:" << std::endl;
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

    Log().get() << "Creating an INSTANCE with the following layers:" << std::endl;
    for (const auto &layer : instanceLayerNames)
    {
        Log().get() << "\t" << layer << std::endl;
    }
    Log().get() << "and with the following extensions:" << std::endl;
    for (const auto &ext : instanceExtensionNames)
    {
        Log().get() << "\t" << ext << std::endl;
    }

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
    VK_ERR_CHECK(createInstanceFn(&createInfo, nullptr, &instance));
    this->_function_table.get_instance_functions(instance);
    this->_instance = { instance, this->_function_table.destroy_instance_wrapper };
}

void
Renderer::Impl::init_debug_callback()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;

    auto instance = this->_instance.get();

    auto create_debug_report_cb_fn = GETIFN(instance, vkCreateDebugReportCallbackEXT);
    if (nullptr == create_debug_report_cb_fn)
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
    VK_ERR_CHECK(create_debug_report_cb_fn(
        instance,
        &createInfo,
        nullptr,
        &callback
    ));
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
    if (VK_DEBUG_REPORT_ERROR_BIT_EXT == (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT))
    {
        return VK_TRUE;
    }
    return VK_FALSE;
}

void
Renderer::Impl::create_window_surface()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto instance = this->_instance.get();

#if defined(D_BAM_PLATFORM_WINDOWS)
    ::VkWin32SurfaceCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = this->_window->getNativeWindowHandle();
    createInfo.hinstance = ::GetModuleHandle(nullptr);

    auto createWindowSurfaceFn = GETIFN(instance, vkCreateWin32SurfaceKHR);
    ::VkSurfaceKHR surface;
    VK_ERR_CHECK(createWindowSurfaceFn(
        instance,
        &createInfo,
        nullptr,
        &surface
    ));

    auto surfaceDeleter = [instance](::VkSurfaceKHR inSurface)
    {
        auto destroy = GETIFN(instance, vkDestroySurfaceKHR);
        Log().get() << "Destroying VkSurfaceKHR 0x" << std::hex << inSurface << std::endl;
        destroy(instance, inSurface, nullptr);
    };
    this->_surface = std::unique_ptr<::VkSurfaceKHR_T, decltype(surfaceDeleter)>(surface, surfaceDeleter);
#elif defined(D_BAM_PLATFORM_OSX)
    ::VkMacOSSurfaceCreateInfoMVK createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = this->_window->macosGetViewHandle();

    auto createWindowSurfaceFn = GETIFN(instance, vkCreateMacOSSurfaceMVK);
    ::VkSurfaceKHR surface;
    VK_ERR_CHECK(createWindowSurfaceFn(
        instance,
        &createInfo,
        nullptr,
        &surface
    ));

    this->_surface = { surface, this->_function_table.destroy_surface_khr_wrapper };
#else
#error Unsupported platform
#endif
}

void
Renderer::Impl::enumerate_physical_devices()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto instance = this->_instance.get();

    // enumerate physical devices
    auto enumPhysDevicesFn = GETIFN(instance, vkEnumeratePhysicalDevices);
    uint32_t numPhysicalDevices = 0;

    // get number of physical devices
    VK_ERR_CHECK(enumPhysDevicesFn(instance, &numPhysicalDevices, nullptr));
    if (0 == numPhysicalDevices)
    {
        throw Exception("There are no physical devices available on this hardware");
    }
    Log().get() << "Found " << numPhysicalDevices << " physical devices" << std::endl;
    this->_physical_devices.resize(numPhysicalDevices);
    VK_ERR_CHECK(enumPhysDevicesFn(instance, &numPhysicalDevices, this->_physical_devices.data()));

    auto enumeratePhysicalDevicePropsFn = GETIFN(instance, vkGetPhysicalDeviceProperties);
    auto enumeratePhysicalMemoryDeviceFn = GETIFN(instance, vkGetPhysicalDeviceMemoryProperties);
    auto getPhysDeviceFeaturesFn = GETIFN(instance, vkGetPhysicalDeviceFeatures);
    for (auto pdevice_index = 0u; pdevice_index < numPhysicalDevices; ++pdevice_index)
    {
        auto device = this->_physical_devices[pdevice_index];
        Log().get() << "PHYSICAL DEVICE " << pdevice_index << std::endl;
        // enumerate physical device properties
        {
            ::VkPhysicalDeviceProperties props;
            enumeratePhysicalDevicePropsFn(device, &props);
            Log().get() << "\tProperties of physical device" << std::endl;
            Log().get() << "\t\tAPI version : " << props.apiVersion << std::endl;
            Log().get() << "\t\tDevice ID : " << props.deviceID << std::endl;
            Log().get() << "\t\tDevice name : " << props.deviceName << std::endl;
            Log().get() << "\t\tDevice type : " << props.deviceType << std::endl;
            Log().get() << "\t\tDriver version : " << props.driverVersion << std::endl;
            Log().get() << "\t\tPipeline Cache UUID : " << props.pipelineCacheUUID << std::endl;
            Log().get() << "\t\tLimits... TODO" << std::endl;
            Log().get() << "\t\tSparse properties... TODO" << std::endl;
        }
        // enumerate physical device memory properties
        {
            ::VkPhysicalDeviceMemoryProperties memProps;
            enumeratePhysicalMemoryDeviceFn(device, &memProps);
            Log().get() << "\tMemory properties of physical device" << std::endl;
            Log().get() << "\t\tMemory heap count : " << memProps.memoryHeapCount << std::endl;
            for (auto i = 0u; i < memProps.memoryHeapCount; ++i)
            {
                Log().get() << "\t\t\tHeap " << i << std::endl;
                Log().get() << "\t\t\t\tFlags : " << to_string(static_cast<::VkMemoryHeapFlagBits>(memProps.memoryHeaps[i].flags)) << std::endl;
                const auto bytes = memProps.memoryHeaps[i].size;
                const auto kbytes = bytes / 1024;
                const auto mbytes = kbytes / 1024;
                const auto gbytes = mbytes / 1024;
                Log().get() << "\t\t\t\tSize : " << bytes << " bytes / " << kbytes << " KB / " << mbytes << " MB / " << gbytes << " GB" << std::endl;
            }
            Log().get() << "\t\tMemory type count : " << memProps.memoryTypeCount << std::endl;
            for (auto i = 0u; i < memProps.memoryTypeCount; ++i)
            {
                Log().get() << "\t\t\tMemory type " << i << std::endl;
                Log().get() << "\t\t\t\tHeap index : " << memProps.memoryTypes[i].heapIndex << std::endl;
                Log().get() << "\t\t\t\tProperties : " << to_string(static_cast<::VkMemoryPropertyFlagBits>(memProps.memoryTypes[i].propertyFlags)) << std::endl;
            }
        }
        // enumerate physical device features
        {
            VkPhysicalDeviceFeatures features;
            getPhysDeviceFeaturesFn(device, &features);
            Log().get() << "\tFeatures of physical device" << std::endl;

#define LOG_FEATURE(_feature) Log().get() << "\t\t"#_feature << ": " << features._feature << std::endl

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
    }

    // arbitrary choice
    this->_physical_device_index = 0;
    Log().get() << "Choosing PHYSICAL device " << this->_physical_device_index << std::endl;
}

void
Renderer::Impl::create_logical_device()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto instance = this->_instance.get();
    auto pDevice = this->_physical_devices[this->_physical_device_index];
    auto surface = this->_surface.get();

    // enumerate physical device extensions
    auto enumDeviceExtensionPropertiesFn = GETIFN(instance, vkEnumerateDeviceExtensionProperties);
    uint32_t numDeviceExtensions = 0;
    VK_ERR_CHECK(enumDeviceExtensionPropertiesFn(pDevice, nullptr, &numDeviceExtensions, nullptr));
    std::vector<::VkExtensionProperties> deviceExtensions(numDeviceExtensions);
    VK_ERR_CHECK(enumDeviceExtensionPropertiesFn(pDevice, nullptr, &numDeviceExtensions, deviceExtensions.data()));
    Log().get() << "Found the following " << numDeviceExtensions << " DEVICE extensions:" << std::endl;
    for (auto i = 0u; i < numDeviceExtensions; ++i)
    {
        const auto &ext = deviceExtensions[i];
        Log().get() << "\t" << i << ": " << ext.extensionName << ", " << ext.specVersion << std::endl;
    }

    // enumerate physical device layers
    auto enumDeviceLayerPropertiesFn = GETIFN(instance, vkEnumerateDeviceLayerProperties);
    uint32_t numDeviceLayers = 0;
    VK_ERR_CHECK(enumDeviceLayerPropertiesFn(pDevice, &numDeviceLayers, nullptr));
    std::vector<::VkLayerProperties> deviceLayers(numDeviceLayers);
    VK_ERR_CHECK(enumDeviceLayerPropertiesFn(pDevice, &numDeviceLayers, deviceLayers.data()));
    Log().get() << "Found the following " << numDeviceLayers << " DEVICE layers:" << std::endl;
    for (auto i = 0u; i < numDeviceLayers; ++i)
    {
        const auto &layer = deviceLayers[i];
        Log().get() << "\t" << i << ": " << layer.layerName << ", " << layer.description << ", " << layer.implementationVersion << ", " << layer.specVersion << std::endl;
    }

    // query the family of queues available
    auto getPDeviceQueueFamilyPropsFn = GETIFN(instance, vkGetPhysicalDeviceQueueFamilyProperties);
    uint32_t numQueueFamilyProperties = 0;
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, nullptr);
    if (0 == numQueueFamilyProperties)
    {
        throw Exception("Unable to find any queue families on this physical device");
    }
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilyProperties);
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, queueFamilyProperties.data());
    Log().get() << "Found the following " << numQueueFamilyProperties << " QUEUE FAMILIES on physical device " << this->_physical_device_index << std::endl;
    for (auto i = 0u; i < numQueueFamilyProperties; ++i)
    {
        Log().get() << "\tQueue family " << i << std::endl;
        Log().get() << "\t\tFlags : " << to_string(static_cast<::VkQueueFlagBits>(queueFamilyProperties[i].queueFlags)) << std::endl;
        Log().get() << "\t\tCount : " << queueFamilyProperties[i].queueCount << std::endl;
        Log().get() << "\t\tTimestampValidBits : " << queueFamilyProperties[i].timestampValidBits << std::endl;
    }

    // assume that the first queue family is capable of graphics
    auto graphics_family_queue_index = 0;
    // assume that the same queue family is capable of presentation
    auto present_family_queue_index = 0;

    if (0 == (queueFamilyProperties[graphics_family_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
        throw Exception("Unable to find queue family with graphics support on this physical device");
    }

    ::VkBool32 presentSupport = false;
    auto getPDeviceSurfaceSupportFn = GETIFN(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_ERR_CHECK(getPDeviceSurfaceSupportFn(
        pDevice,
        present_family_queue_index,
        surface,
        &presentSupport
    ));
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

    Log().get() << "Creating a LOGICAL DEVICE with the following layers:" << std::endl;
    for (const auto &layer : deviceLayersRequired)
    {
        Log().get() << "\t" << layer << std::endl;
    }
    Log().get() << "and with the following extensions:" << std::endl;
    for (const auto &ext : deviceExtensionsRequired)
    {
        Log().get() << "\t" << ext << std::endl;
    }

    // create a logical device
    auto createDeviceFn = GETIFN(instance, vkCreateDevice);
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
    VK_ERR_CHECK(createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &device));
    this->_logical_device = { device, this->_function_table.destroy_device_wrapper };

    auto logical_device = this->_logical_device.get();
    this->_function_table.get_device_functions(logical_device);

    auto getQueueFn = GETDFN(logical_device, vkGetDeviceQueue);
    auto graphics_queue_index = 0;
    getQueueFn(
        logical_device,
        graphics_family_queue_index,
        graphics_queue_index,
        &this->_graphics_queue
    );
    auto present_queue_index = 0;
    getQueueFn(
        logical_device,
        present_family_queue_index,
        present_queue_index,
        &this->_present_queue
    );
}

void
Renderer::Impl::create_swapchain()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto instance = this->_instance.get();
    auto pDevice = this->_physical_devices[this->_physical_device_index];
    auto surface = this->_surface.get();

    auto getSurfaceCapsFn = GETIFN(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    ::VkSurfaceCapabilitiesKHR surfaceCaps;
    VK_ERR_CHECK(getSurfaceCapsFn(
        pDevice,
        surface,
        &surfaceCaps
    ));
    Log().get() << "Surface capabilities" << std::endl;
    Log().get() << "\tImage count: [" << surfaceCaps.minImageCount << ", " << surfaceCaps.maxImageCount << "]" << std::endl;
    Log().get() << "\tImage extent: [(" << surfaceCaps.minImageExtent.width << "x" << surfaceCaps.minImageExtent.height << "), (" << surfaceCaps.maxImageExtent.width << "x" << surfaceCaps.maxImageExtent.height << ")]" << std::endl;

    uint32_t surfaceFormatCount = 0;
    auto getSurfaceFormatsFn = GETIFN(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_ERR_CHECK(getSurfaceFormatsFn(
        pDevice,
        surface,
        &surfaceFormatCount,
        nullptr
    ));
    std::vector<::VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VK_ERR_CHECK(getSurfaceFormatsFn(
        pDevice,
        surface,
        &surfaceFormatCount,
        surfaceFormats.data()
    ));
    Log().get() << "Found " << surfaceFormatCount << " SURFACE FORMATS:" << std::endl;
    for (const auto format : surfaceFormats)
    {
        Log().get() << "\t" << format.format << " " << format.colorSpace << std::endl;
    }

    uint32_t presentModeCount = 0;
    auto getPresentModesFn = GETIFN(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_ERR_CHECK(getPresentModesFn(
        pDevice,
        surface,
        &presentModeCount,
        nullptr
    ));
    std::vector<::VkPresentModeKHR> presentModes(presentModeCount);
    VK_ERR_CHECK(getPresentModesFn(
        pDevice,
        surface,
        &presentModeCount,
        presentModes.data()
    ));
    Log().get() << "Found " << presentModeCount << " PRESENT MODES:" << std::endl;
    for (const auto pMode : presentModes)
    {
        Log().get() << "\t" << pMode << std::endl;
    }

    this->_swapchain_imageFormat = surfaceFormats[0].format;
    this->_swapchain_extent = surfaceCaps.maxImageExtent;

    auto logical_device = this->_logical_device.get();
    ::VkSwapchainCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = surfaceCaps.minImageCount;
    createInfo.imageFormat = this->_swapchain_imageFormat;
    createInfo.imageColorSpace = surfaceFormats[0].colorSpace;
    createInfo.imageExtent = this->_swapchain_extent;
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
    auto createSwapchainFn = GETDFN(logical_device, vkCreateSwapchainKHR);
    VK_ERR_CHECK(createSwapchainFn(
        logical_device,
        &createInfo,
        nullptr,
        &swapchain
    ));
    this->_swapchain = { swapchain, this->_function_table.destroy_swapchain_khr_wrapper };

    auto getswapchainimagesFn = GETDFN(logical_device, vkGetSwapchainImagesKHR);
    uint32_t swapchain_imagecount = 0;
    VK_ERR_CHECK(getswapchainimagesFn(
        logical_device,
        this->_swapchain.get(),
        &swapchain_imagecount,
        nullptr
    ));
    this->_swapchain_images.resize(swapchain_imagecount);
    VK_ERR_CHECK(getswapchainimagesFn(
        logical_device,
        this->_swapchain.get(),
        &swapchain_imagecount,
        this->_swapchain_images.data()
    ));
}

void
Renderer::Impl::create_imageviews()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto logical_device = this->_logical_device.get();

    // no resize of this->_swapchain_imageViews, since there is no valid
    // default constructor of a std::unique_ptr with a custom deleter
    auto createImageViewFn = GETDFN(logical_device, vkCreateImageView);
    for (auto i = 0u; i < this->_swapchain_images.size(); ++i)
    {
        ::VkImageViewCreateInfo createInfo;
        memset(&createInfo, 0, sizeof(createInfo));
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.image = this->_swapchain_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = this->_swapchain_imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        ::VkImageView view;
        VK_ERR_CHECK(createImageViewFn(
            logical_device,
            &createInfo,
            nullptr,
            &view
        ));
        this->_swapchain_imageViews.emplace_back(view, this->_function_table.destroy_imageview_wrapper);
    }
}

void
Renderer::Impl::create_renderpass()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    ::VkAttachmentDescription colorAttachment;
    memset(&colorAttachment, 0, sizeof(colorAttachment));
    colorAttachment.format = this->_swapchain_imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    ::VkAttachmentReference colorAttachmentRef;
    memset(&colorAttachmentRef, 0, sizeof(colorAttachmentRef));
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ::VkSubpassDescription subpass;
    memset(&subpass, 0, sizeof(subpass));
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    ::VkSubpassDependency dependency;
    memset(&dependency, 0, sizeof(dependency));
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    ::VkRenderPassCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &colorAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    auto logical_device = this->_logical_device.get();
    auto createRenderPassFn = GETDFN(logical_device , vkCreateRenderPass);
    ::VkRenderPass renderPass;
    VK_ERR_CHECK(createRenderPassFn(
        logical_device,
        &createInfo,
        nullptr,
        &renderPass
    ));
    this->_renderPass = { renderPass, this->_function_table.destroy_renderpass_wrapper };
}

void
Renderer::Impl::create_framebuffers()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto logical_device = this->_logical_device.get();
    // no resize of this->_framebuffers since there is no default constructor for
    // std::unique_ptr with a custom deleter
    auto createFrameBufferFn = GETDFN(logical_device, vkCreateFramebuffer);
    for (auto i = 0u; i < this->_swapchain_images.size(); ++i)
    {
        ::VkImageView attachments[] = { this->_swapchain_imageViews[i].get() };

        ::VkFramebufferCreateInfo createInfo;
        memset(&createInfo, 0, sizeof(createInfo));
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = this->_renderPass.get();
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = this->_swapchain_extent.width;
        createInfo.height = this->_swapchain_extent.height;
        createInfo.layers = 1;

        ::VkFramebuffer frameBuffer;
        VK_ERR_CHECK(createFrameBufferFn(
            logical_device,
            &createInfo,
            nullptr,
            &frameBuffer
        ));
        this->_framebuffers.emplace_back(frameBuffer, this->_function_table.destroy_framebuffer_wrapper);
    }
}

void
Renderer::Impl::create_commandpool()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    ::VkCommandPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = 0; // TODO: hook up
    createInfo.flags = 0;

    auto logical_device = this->_logical_device.get();
    auto createCommandPoolFn = GETDFN(logical_device, vkCreateCommandPool);
    ::VkCommandPool commandPool;
    VK_ERR_CHECK(createCommandPoolFn(
        logical_device,
        &createInfo,
        nullptr,
        &commandPool
    ));
    this->_commandPool = { commandPool, this->_function_table.destroy_commandpool_wrapper };
}

void
Renderer::Impl::create_commandbuffers()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    this->_commandBuffers.resize(this->_swapchain_images.size());

    ::VkCommandBufferAllocateInfo allocateInfo;
    memset(&allocateInfo, 0, sizeof(allocateInfo));
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = this->_commandPool.get();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<uint32_t>(this->_commandBuffers.size());

    auto logical_device = this->_logical_device.get();
    auto allocateCommandBuffersFn = GETDFN(logical_device, vkAllocateCommandBuffers);
    VK_ERR_CHECK(allocateCommandBuffersFn(
        logical_device,
        &allocateInfo,
        this->_commandBuffers.data()
    ));

    auto beginCommandBufferFn = GETDFN(logical_device , vkBeginCommandBuffer);
    auto cmdBeginRenderPassFn = GETDFN(logical_device, vkCmdBeginRenderPass);
    auto cmdEndRenderPassFn = GETDFN(logical_device, vkCmdEndRenderPass);
    auto endCommandBufferFn = GETDFN(logical_device, vkEndCommandBuffer);
    for (auto i = 0u; i < this->_commandBuffers.size(); ++i)
    {
        ::VkCommandBufferBeginInfo beginInfo;
        memset(&beginInfo, 0, sizeof(beginInfo));
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VK_ERR_CHECK(beginCommandBufferFn(
            this->_commandBuffers[i],
            &beginInfo
        ));

        ::VkRenderPassBeginInfo renderPassInfo;
        memset(&renderPassInfo, 0, sizeof(renderPassInfo));
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = this->_renderPass.get();
        renderPassInfo.framebuffer = this->_framebuffers[i].get();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = this->_swapchain_extent;

        ::VkClearValue clearColour;
        if (0 == i)
        {
            clearColour.color = {{1, 0, 0, 1}};
        }
        else
        {
            clearColour.color = {{0, 0, 1, 1}};
        }
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColour;

        cmdBeginRenderPassFn(
            this->_commandBuffers[i],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        // intentionally empty for now

        cmdEndRenderPassFn(
            this->_commandBuffers[i]
        );

        VK_ERR_CHECK(endCommandBufferFn(
            this->_commandBuffers[i]
        ));
    }
}

void
Renderer::Impl::create_semaphores()
{
    Log().get() << "==================================================" << std::endl;
    Log().get() << "## " << __FUNCTION__ << std::endl;
    Log().get() << "==================================================" << std::endl;
    auto logical_device = this->_logical_device.get();

    ::VkSemaphoreCreateInfo semaphoreCreateInfo;
    memset(&semaphoreCreateInfo, 0, sizeof(semaphoreCreateInfo));
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    ::VkFenceCreateInfo fenceCreateInfo;
    memset(&fenceCreateInfo, 0, sizeof(fenceCreateInfo));
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    auto createSemaphoreFn = GETDFN(logical_device, vkCreateSemaphore);
    ::VkSemaphore sem;

    auto createFenceFn = GETDFN(logical_device, vkCreateFence);
    ::VkFence fence;

    for (auto i = 0u; i < this->MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VK_ERR_CHECK(createSemaphoreFn(
            logical_device,
            &semaphoreCreateInfo,
            nullptr,
            &sem
        ));
        this->_image_available.emplace_back(sem, this->_function_table.destroy_semaphore_wrapper);

        VK_ERR_CHECK(createSemaphoreFn(
            logical_device,
            &semaphoreCreateInfo,
            nullptr,
            &sem
        ));
        this->_render_finished.emplace_back(sem, this->_function_table.destroy_semaphore_wrapper);

        VK_ERR_CHECK(createFenceFn(
            logical_device,
            &fenceCreateInfo,
            nullptr,
            &fence
        ));
        this->_inflight_fence.emplace_back(fence, this->_function_table.destroy_fence_wrapper);
    }
}

#define LOG_FLAG(_type,_flag) \
if (inFlags & _flag)\
{\
    stream << #_flag;\
    inFlags = static_cast<_type>(inFlags & ~_flag);\
}

std::string
Renderer::Impl::to_string(
    ::VkMemoryPropertyFlagBits inFlags)
{
    std::stringstream stream;
    while (inFlags != 0)
    {
        LOG_FLAG(::VkMemoryPropertyFlagBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        else LOG_FLAG(::VkMemoryPropertyFlagBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        else LOG_FLAG(::VkMemoryPropertyFlagBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        else LOG_FLAG(::VkMemoryPropertyFlagBits, VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        else LOG_FLAG(::VkMemoryPropertyFlagBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        else
        {
            throw Exception("Unknown memory property bit");
        }
        stream << " | ";
    }
    return stream.str();
}

std::string
Renderer::Impl::to_string(
    ::VkMemoryHeapFlagBits inFlags)
{
    std::stringstream stream;
    while (inFlags != 0)
    {
        LOG_FLAG(::VkMemoryHeapFlagBits, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        else
        {
            throw Exception("Unknown memory heap bit");
        }
        stream << " | ";
    }
    return stream.str();
}

std::string
Renderer::Impl::to_string(
    ::VkQueueFlagBits inFlags)
{
    std::stringstream stream;
    while (inFlags != 0)
    {
        LOG_FLAG(::VkQueueFlagBits, VK_QUEUE_GRAPHICS_BIT)
        else LOG_FLAG(::VkQueueFlagBits, VK_QUEUE_COMPUTE_BIT)
        else LOG_FLAG(::VkQueueFlagBits, VK_QUEUE_TRANSFER_BIT)
        else LOG_FLAG(::VkQueueFlagBits, VK_QUEUE_SPARSE_BINDING_BIT)
        else
        {
            throw Exception("Unknown queue bit");
        }
        stream << " | ";
    }
    return stream.str();
}

#undef LOG_FLAG
