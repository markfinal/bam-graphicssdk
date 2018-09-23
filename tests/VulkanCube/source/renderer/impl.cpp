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

#include <algorithm>
#include <array>
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

    auto khr_surface_it = std::find_if(extensions.begin(), extensions.end(), [](::VkExtensionProperties &extension)
    {
        return (0 == strcmp(extension.extensionName, "VK_KHR_surface"));
    });
    if (khr_surface_it == extensions.end())
    {
        throw Exception("Instance does not support the VK_KHR_surface extension");
    }

    const std::array<const char*, 1> instanceExtensionNames
    {
        { "VK_KHR_surface" }
    };

    ::VkInstanceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // required
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = instanceExtensionNames.size();
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
    Log().get() << "Found " << numPhysicalDevices << " physical devices" << std::endl;
    this->_physical_devices.resize(numPhysicalDevices);
    enumPhysDevicesRes = enumPhysDevicesFn(this->_instance.get(), &numPhysicalDevices, this->_physical_devices.data());
    if (VK_SUCCESS != enumPhysDevicesRes)
    {
        throw Exception("Unable to enumerate physical devices");
    }

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

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(numQueueFamilyProperties);
    getPDeviceQueueFamilyPropsFn(pDevice, &numQueueFamilyProperties, queueFamilyProperties.data());
    if (0 == (queueFamilyProperties[graphics_family_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    {
        throw Exception("Unable to find queue family with graphics support on this physical device");
    }

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
    ::VkDevice device;
    auto createDeviceRes = createDeviceFn(pDevice, &deviceCreateInfo, nullptr, &device);
    if (VK_SUCCESS != createDeviceRes)
    {
        throw Exception("Unable to find create logical device");
    }
    this->_logical_device = { device, this->_function_table.destroy_device_wrapper };

    auto graphics_queue_index = 0;

    auto getQueueFn = GETIFN(this->_instance.get(), vkGetDeviceQueue);
    getQueueFn(
        this->_logical_device.get(),
        graphics_family_queue_index,
        graphics_queue_index,
        &this->_graphics_queue
    );
}
