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
    std::unique_ptr< ::VkInstance_T, void(*)(::VkInstance)>                              _instance;
    std::unique_ptr< ::VkDebugReportCallbackEXT_T, void(*)(::VkDebugReportCallbackEXT)>  _debug_callback;
    AppWindow                                                                           *_window = nullptr;
    std::unique_ptr< ::VkSurfaceKHR_T, void(*)(::VkSurfaceKHR)>                          _surface;
    std::vector< ::VkPhysicalDevice>                                                     _physical_devices;
    size_t                                                                               _physical_device_index = static_cast<size_t>(-1);
    std::unique_ptr< ::VkDevice_T, void(*)(::VkDevice)>                                  _logical_device;
    ::VkQueue                                                                            _graphics_queue;
    ::VkQueue                                                                            _present_queue;
    ::VkFormat                                                                           _swapchain_imageFormat;
    ::VkExtent2D                                                                         _swapchain_extent;
    std::unique_ptr<::VkSwapchainKHR_T, void(*)(::VkSwapchainKHR)>                       _swapchain;
    std::vector<::VkImage>                                                               _swapchain_images;
    std::unique_ptr<::VkImageView_T, void(*)(::VkImageView)>                             _swapchain_imageView1;
    std::unique_ptr<::VkImageView_T, void(*)(::VkImageView)>                             _swapchain_imageView2;
    std::unique_ptr<::VkRenderPass_T, void(*)(::VkRenderPass)>                           _renderPass;
    std::unique_ptr<::VkFramebuffer_T, void(*)(::VkFramebuffer)>                         _framebuffer1;
    std::unique_ptr<::VkFramebuffer_T, void(*)(::VkFramebuffer)>                         _framebuffer2;
    std::unique_ptr<::VkCommandPool_T, void(*)(::VkCommandPool)>                         _commandPool;
    std::vector<::VkCommandBuffer>                                                       _commandBuffers;

    class VkFunctionTable
    {
    private:
        static PFN_vkDestroyInstance   _destroy_instance;
        static PFN_vkDestroyDebugReportCallbackEXT _destroy_debug_callback;
        static std::function<void(::VkDebugReportCallbackEXT, const ::VkAllocationCallbacks*)> _destroy_debug_callback_boundinstance;
        static PFN_vkDeviceWaitIdle    _device_waitidle;
        static PFN_vkDestroyDevice     _destroy_device;
        static PFN_vkDestroySurfaceKHR _destroy_surface_khr;
        static std::function<void(::VkSurfaceKHR, const ::VkAllocationCallbacks*)> _destroy_surface_khr_boundinstance;
        static PFN_vkDestroySwapchainKHR _destroy_swapchain_khr;
        static std::function<void(::VkSwapchainKHR, const ::VkAllocationCallbacks*)> _destroy_swapchain_khr_bounddevice;
        static PFN_vkDestroyImageView _destroy_imageview;
        static std::function<void(::VkImageView, const ::VkAllocationCallbacks*)> _destroy_imageview_bounddevice;
        static PFN_vkDestroyRenderPass _destroy_renderpass;
        static std::function<void(::VkRenderPass, const ::VkAllocationCallbacks*)> _destroy_renderpass_bounddevice;
        static PFN_vkDestroyFramebuffer _destroy_framebuffer;
        static std::function<void(::VkFramebuffer, const ::VkAllocationCallbacks*)> _destroy_framebuffer_bounddevice;
        static PFN_vkDestroyCommandPool _destroy_commandpool;
        static std::function<void(::VkCommandPool, const ::VkAllocationCallbacks*)> _destroy_commandpool_bounddevice;

    public:
        static void
        get_instance_functions(
            ::VkInstance inInstance);

        static void
        get_device_functions(
            ::VkInstance inInstance,
            ::VkDevice inDevice);

        static void
        destroy_instance_wrapper(
            ::VkInstance inInstance);

        static void
        destroy_debug_callback_wrapper(
            ::VkDebugReportCallbackEXT inDebugCallback);

        static void
        destroy_device_wrapper(
            ::VkDevice inDevice);

        static void
        destroy_surface_khr_wrapper(
            ::VkSurfaceKHR inSurface);

        static void
        destroy_swapchain_khr_wrapper(
            ::VkSwapchainKHR inSwapchain);

        static void
        destroy_imageview_wrapper(
            ::VkImageView inImageView);

        static void
        destroy_renderpass_wrapper(
            ::VkRenderPass inRenderPass);

        static void
        destroy_framebuffer_wrapper(
            ::VkFramebuffer inFrameBuffer);

        static void
        destroy_commandpool_wrapper(
            ::VkCommandPool inCommandPool);
    };
    VkFunctionTable                                        _function_table;

    Impl(
        AppWindow *inWindow);
    ~Impl();

    void
    create_instance();

    void
    init_debug_callback();

    static VkBool32
    debug_callback(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData);

    void
    create_window_surface();

    void
    enumerate_physical_devices();

    void
    create_logical_device();

    void
    create_swapchain();

    void
    create_imageviews();

    void
    create_renderpass();

    void
    create_framebuffers();

    void
    create_commandpool();

    void
    create_commandbuffers();

    static std::string
    to_string(
        ::VkMemoryPropertyFlagBits inFlags);

    static std::string
    to_string(
        ::VkMemoryHeapFlagBits inFlags);

    static std::string
    to_string(
        ::VkQueueFlagBits inFlags);
};

#endif // VULKAN_RENDERER_IMPL_H
