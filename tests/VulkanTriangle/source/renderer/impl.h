/*
Copyright (c) 2010-2019, Mark Final
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
#include <cassert>
#include <string>

// these macros avoid repetition between stating the name of the function and the PFN_* type
#define GETPFN(_name) PFN_##_name
#define GETFN(_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(nullptr, #_name))
#define GETIFN(_instance,_name) reinterpret_cast<GETPFN(_name)>(vkGetInstanceProcAddr(_instance, #_name))
#define GETDFN(_logical_device,_name) reinterpret_cast<GETPFN(_name)>(vkGetDeviceProcAddr(_logical_device, #_name))

#ifdef NDEBUG
# define VK_ERR_CHECK(_fn_call) _fn_call
# define VK_ERR_CHECK_QUIET(_fn_call) _fn_call
#else // NDEBUG
# define VK_ERR_CHECK(_fn_call) \
do{\
auto result = _fn_call;\
if (VK_SUCCESS != result) { Log().get() << "FAILED (" << result << "): " <<  #_fn_call << std::endl; } \
else { Log().get() << "SUCCESS: " << #_fn_call << std::endl; } \
assert(VK_SUCCESS == result);\
} while(0)
# define VK_ERR_CHECK_QUIET(_fn_call) \
do{\
auto result = _fn_call;\
assert(VK_SUCCESS == result);\
} while(0)
#endif // NDEBUG

struct Renderer::Impl
{
    const uint32_t MAX_FRAMES_IN_FLIGHT = 2u;

    std::unique_ptr<::VkInstance_T, std::function<void(::VkInstance)>>                             _instance;
    std::unique_ptr<::VkDebugReportCallbackEXT_T, std::function<void(::VkDebugReportCallbackEXT)>> _debug_callback;
    AppWindow                                                                                     *_window = nullptr;
    std::unique_ptr< ::VkSurfaceKHR_T, std::function<void(::VkSurfaceKHR)>>                        _surface;
    std::vector< ::VkPhysicalDevice>                                                               _physical_devices;
    size_t                                                                                         _physical_device_index = static_cast<size_t>(-1);
    std::unique_ptr< ::VkDevice_T, std::function<void(::VkDevice)>>                                _logical_device;
    ::VkQueue                                                                                      _graphics_queue;
    ::VkQueue                                                                                      _present_queue;
    ::VkFormat                                                                                     _swapchain_imageFormat;
    ::VkExtent2D                                                                                   _swapchain_extent;
    std::unique_ptr<::VkSwapchainKHR_T, std::function<void(::VkSwapchainKHR)>>                     _swapchain;
    std::vector<::VkImage>                                                                         _swapchain_images;
    std::vector<std::unique_ptr<::VkImageView_T, std::function<void(::VkImageView)>>>              _swapchain_imageViews;
    std::unique_ptr<::VkShaderModule_T, std::function<void(::VkShaderModule)>>                     _vert_shader_module;
    std::unique_ptr<::VkShaderModule_T, std::function<void(::VkShaderModule)>>                     _frag_shader_module;
    std::unique_ptr<::VkRenderPass_T, std::function<void(::VkRenderPass)>>                         _renderPass;
    std::unique_ptr<::VkPipelineLayout_T, std::function<void(::VkPipelineLayout)>>                 _pipeline_layout;
    std::unique_ptr<::VkPipeline_T, std::function<void(::VkPipeline)>>                             _pipeline;
    std::vector<std::unique_ptr<::VkFramebuffer_T, std::function<void(::VkFramebuffer)>>>          _framebuffers;
    std::unique_ptr<::VkCommandPool_T, std::function<void(::VkCommandPool)>>                       _commandPool;
    std::vector<::VkCommandBuffer>                                                                 _commandBuffers;
    std::vector<std::unique_ptr<::VkSemaphore_T, std::function<void(::VkSemaphore)>>>              _image_available;
    std::vector<std::unique_ptr<::VkSemaphore_T, std::function<void(::VkSemaphore)>>>              _render_finished;
    std::vector<std::unique_ptr<::VkFence_T, std::function<void(::VkFence)>>>                      _inflight_fence;
    uint32_t                                                                                       _current_frame = 0;

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
    create_graphics_pipeline();

    void
    create_renderpass();

    void
    create_framebuffers();

    void
    create_commandpool();

    void
    create_commandbuffers();

    void
    create_semaphores();

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
