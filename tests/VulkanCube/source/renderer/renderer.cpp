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

#include "../appwindow.h"

Renderer::Renderer(
    AppWindow *inWindow)
    :
    _impl(new Impl(inWindow))
{}

Renderer::~Renderer() = default;

void
Renderer::init()
{
    auto impl = this->_impl.get();
    impl->create_instance();
    impl->init_debug_callback();
    impl->create_window_surface();
    impl->enumerate_physical_devices();
    impl->create_logical_device();
    impl->create_swapchain();
    impl->create_imageviews();
    impl->create_renderpass();
    impl->create_framebuffers();
    impl->create_commandpool();
    impl->create_commandbuffers();
    impl->create_semaphores();
}

void
Renderer::draw_frame() const
{
    auto impl = this->_impl.get();
    auto acquireNextImageFn = GETIFN(impl->_instance.get(), vkAcquireNextImageKHR);
    uint32_t imageIndex;
    VK_ERR_CHECK(acquireNextImageFn(
        impl->_logical_device.get(),
        impl->_swapchain.get(),
        std::numeric_limits<uint64_t>::max(),
        impl->_image_available.get(),
        VK_NULL_HANDLE,
        &imageIndex
    ));

    ::VkSemaphore waitSemaphores[] = { impl->_image_available.get() };
    ::VkSemaphore signalSemaphores[] = { impl->_render_finished.get() };
    ::VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    ::VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &impl->_commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    auto queueSubmitFn = GETIFN(impl->_instance.get(), vkQueueSubmit);
    VK_ERR_CHECK(queueSubmitFn(
        impl->_graphics_queue,
        1,
        &submitInfo,
        VK_NULL_HANDLE
    ));

    ::VkSwapchainKHR swapchains[] = { impl->_swapchain.get() };
    ::VkPresentInfoKHR presentInfo;
    memset(&presentInfo, 0, sizeof(presentInfo));
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    auto queuePresentFn = GETIFN(impl->_instance.get(), vkQueuePresentKHR);
    VK_ERR_CHECK(queuePresentFn(
        impl->_present_queue,
        &presentInfo
    ));
}
