#ifndef VULKAN_RENDERER_IMPL_H
#define VULKAN_RENDERER_IMPL_H

#include "renderer.h"
#include "vulkan/vulkan.h"

#include <vector>

// these macros avoid repetition between stating the name of the function and the PFN_* type
#define GETPFN(_name) PFN_##_name
#define GETFN(_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(nullptr, #_name))
#define GETIFN(_instance,_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(_instance, #_name))

struct Renderer::Impl
{
    ::VkInstance                    _instance;
    std::vector<::VkPhysicalDevice> _physical_devices;
    size_t                          _physical_device_index = -1;
    ::VkDevice                      _logical_device;

    void
    clean_up();

    void
    create_instance();

    void
    enumerate_physics_devices();

    void
    create_logical_device();
};

#endif // VULKAN_RENDERER_IMPL_H
