/*
** Copyright (c) 2021-2022 LunarG, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
*/

#ifndef GFXRECON_DECODE_VULKAN_VIRTUAL_SWAPCHAIN_H
#define GFXRECON_DECODE_VULKAN_VIRTUAL_SWAPCHAIN_H

#include "decode/vulkan_swapchain.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

class VulkanVirtualSwapchain : public VulkanSwapchain
{
  public:
    virtual ~VulkanVirtualSwapchain() override {}

    virtual VkResult CreateSwapchainKHR(PFN_vkCreateSwapchainKHR        func,
                                        const DeviceInfo*               device_info,
                                        const VkSwapchainCreateInfoKHR* create_info,
                                        const VkAllocationCallbacks*    allocator,
                                        VkSwapchainKHR*                 swapchain,
                                        const VkPhysicalDevice          physical_device,
                                        const encode::InstanceTable*    instance_table,
                                        const encode::DeviceTable*      device_table) override;

    virtual void DestroySwapchainKHR(PFN_vkDestroySwapchainKHR    func,
                                     const DeviceInfo*            device_info,
                                     const SwapchainKHRInfo*      swapchain_info,
                                     const VkAllocationCallbacks* allocator) override;

    virtual VkResult GetSwapchainImagesKHR(PFN_vkGetSwapchainImagesKHR func,
                                           const DeviceInfo*           device_info,
                                           SwapchainKHRInfo*           swapchain_info,
                                           uint32_t                    capture_image_count,
                                           uint32_t*                   image_count,
                                           VkImage*                    images) override;

    virtual VkResult AcquireNextImageKHR(PFN_vkAcquireNextImageKHR func,
                                         const DeviceInfo*         device_info,
                                         SwapchainKHRInfo*         swapchain_info,
                                         uint64_t                  timeout,
                                         SemaphoreInfo*            semaphore_info,
                                         FenceInfo*                fence_info,
                                         uint32_t                  capture_image_index,
                                         uint32_t*                 image_index) override;

    virtual VkResult AcquireNextImageKHR(PFN_vkAcquireNextImageKHR func,
                                         const DeviceInfo*         device_info,
                                         SwapchainKHRInfo*         swapchain_info,
                                         uint64_t                  timeout,
                                         VkSemaphore               semaphore,
                                         VkFence                   fence,
                                         uint32_t                  capture_image_index,
                                         uint32_t*                 image_index) override;

    virtual VkResult AcquireNextImage2KHR(PFN_vkAcquireNextImage2KHR       func,
                                          const DeviceInfo*                device_info,
                                          SwapchainKHRInfo*                swapchain_info,
                                          const VkAcquireNextImageInfoKHR* acquire_info,
                                          uint32_t                         capture_image_index,
                                          uint32_t*                        image_index) override;

    virtual VkResult QueuePresentKHR(PFN_vkQueuePresentKHR                 func,
                                     const std::vector<uint32_t>&          capture_image_indices,
                                     const std::vector<SwapchainKHRInfo*>& swapchain_infos,
                                     const QueueInfo*                      queue_info,
                                     const VkPresentInfoKHR*               present_info) override;

    virtual VkResult CreateRenderPass(PFN_vkCreateRenderPass        func,
                                      const DeviceInfo*             device_info,
                                      const VkRenderPassCreateInfo* create_info,
                                      const VkAllocationCallbacks*  allocator,
                                      VkRenderPass*                 render_pass) override;

    virtual VkResult CreateRenderPass2(PFN_vkCreateRenderPass2        func,
                                       const DeviceInfo*              device_info,
                                       const VkRenderPassCreateInfo2* create_info,
                                       const VkAllocationCallbacks*   allocator,
                                       VkRenderPass*                  render_pass) override;

    virtual void ProcessSetSwapchainImageStateCommand(const DeviceInfo* device_info,
                                                      SwapchainKHRInfo* swapchain_info,
                                                      uint32_t          last_presented_image,
                                                      const std::vector<format::SwapchainImageStateInfo>& image_info,
                                                      const VulkanObjectInfoTable& object_info_table,
                                                      SwapchainImageTracker&       swapchain_image_tracker) override
    {}

  private:
    // VirtualSwapchain-specific private information.
    struct VirtualImage
    {
        VkDeviceMemory                        memory{ VK_NULL_HANDLE };
        VkImage                               image{ VK_NULL_HANDLE };
        VulkanResourceAllocator::MemoryData   memory_allocator_data{ 0 };
        VulkanResourceAllocator::ResourceData resource_allocator_data{ 0 };
    };

    struct PrivateData
    {
        std::vector<VkCommandPool>                copy_command_pools;
        std::vector<std::vector<VkCommandBuffer>> copy_command_buffers;
        std::vector<std::vector<VkSemaphore>>     copy_semaphores;
        std::vector<uint32_t>                     capture_to_replay_index;
        std::vector<VirtualImage>
            virtual_swapchain_images; // Images created by replay, returned in place of the swapchain images.
        std::vector<VkImage> replay_swapchain_images; // The real swapchain images.
    };

    VkResult CreateVirtualSwapchainImage(const DeviceInfo*        device_info,
                                         const VkImageCreateInfo& image_create_info,
                                         VirtualImage&            image);

    // Create an unordered map to associate the private information with a particular Vulkan swapchain
    std::unordered_map<VkSwapchainKHR, PrivateData*> private_data_;
};

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_DECODE_VULKAN_VIRTUAL_SWAPCHAIN_H
