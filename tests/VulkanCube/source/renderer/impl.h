#ifndef VULKAN_RENDERER_IMPL_H
#define VULKAN_RENDERER_IMPL_H

#include "renderer.h"
#include "vulkan/vulkan.h"

#include <vector>
#include <memory>

// these macros avoid repetition between stating the name of the function and the PFN_* type
#define GETPFN(_name) PFN_##_name
#define GETFN(_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(nullptr, #_name))
#define GETIFN(_instance,_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(_instance, #_name))

struct Renderer::Impl
{
    std::unique_ptr< ::VkInstance_T, void(*)(::VkInstance)> _instance;
    std::vector< ::VkPhysicalDevice>                        _physical_devices;
    size_t                                                  _physical_device_index = -1;
    std::unique_ptr< ::VkDevice_T, void(*)(::VkDevice)>     _logical_device;
    ::VkQueue                                               _graphics_queue;

    class VkFunctionTable
    {
    private:
        static PFN_vkDestroyInstance _destroy_instance;
        static PFN_vkDestroyDevice   _destroy_device;

    public:
        static void
        get_instance_functions(
            ::VkInstance inInstance);

        static void
        destroy_instance_wrapper(
            ::VkInstance inInstance);

        static void
        destroy_device_wrapper(
            ::VkDevice inInstance);
    };
    VkFunctionTable                                        _function_table;

    Impl();
    ~Impl();

    void
    create_instance();

    void
    enumerate_physics_devices();

    void
    create_logical_device();
};

#endif // VULKAN_RENDERER_IMPL_H
