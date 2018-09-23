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
#ifndef VULKAN_RENDERER_IMPL_H
#define VULKAN_RENDERER_IMPL_H

#include "renderer.h"
#include "vulkan/vulkan.h"

#include <vector>
#include <memory>
#include <functional>

// these macros avoid repetition between stating the name of the function and the PFN_* type
#define GETPFN(_name) PFN_##_name
#define GETFN(_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(nullptr, #_name))
#define GETIFN(_instance,_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(_instance, #_name))

struct Renderer::Impl
{
    std::unique_ptr< ::VkInstance_T, void(*)(::VkInstance)>      _instance;
    AppWindow                                                   *_window = nullptr;
    std::unique_ptr< ::VkSurfaceKHR_T, void(*)(::VkSurfaceKHR)>  _surface;
    std::vector< ::VkPhysicalDevice>                             _physical_devices;
    size_t                                                       _physical_device_index = -1;
    std::unique_ptr< ::VkDevice_T, void(*)(::VkDevice)>          _logical_device;
    ::VkQueue                                                    _graphics_queue;

    class VkFunctionTable
    {
    private:
        static PFN_vkDestroyInstance   _destroy_instance;
        static PFN_vkDestroyDevice     _destroy_device;
        static PFN_vkDestroySurfaceKHR _destroy_surface_khr;
        static std::function<void(::VkSurfaceKHR, const ::VkAllocationCallbacks*)> _destroy_surface_khr_boundinstance;

    public:
        static void
        get_instance_functions(
            ::VkInstance inInstance);

        static void
        destroy_instance_wrapper(
            ::VkInstance inInstance);

        static void
        destroy_device_wrapper(
            ::VkDevice inDevice);

        static void
        destroy_surface_khr_wrapper(
            ::VkSurfaceKHR inSurface);
    };
    VkFunctionTable                                        _function_table;

    Impl(
        AppWindow *inWindow);
    ~Impl();

    void
    create_instance();

    void
    create_window_surface();

    void
    enumerate_physics_devices();

    void
    create_logical_device();
};

#endif // VULKAN_RENDERER_IMPL_H
