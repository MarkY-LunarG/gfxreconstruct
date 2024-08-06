/*
** Copyright (c) 2018-2023 Valve Corporation
** Copyright (c) 2018-2024 LunarG, Inc.
** Copyright (c) 2023 Advanced Micro Devices, Inc.
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

/*
** This file is generated from the Khronos OpenXR XML API Registry.
**
*/

#ifndef  GFXRECON_GENERATED_OPENXR_STRUCT_INIT_NEXT_CHAIN_H
#define  GFXRECON_GENERATED_OPENXR_STRUCT_INIT_NEXT_CHAIN_H

#ifdef ENABLE_OPENXR_SUPPORT

#include "decode/struct_pointer_decoder.h"
#include "generated/generated_openxr_struct_decoders.h"
#include "util/defines.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)


static void InitializeOutputStructNextImpl(const XrBaseInStructure* in_next, XrBaseOutStructure* output_struct)
{
    while(in_next)
    {
        switch(in_next->type)
        {
            case XR_TYPE_API_LAYER_PROPERTIES:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrApiLayerProperties>());
                break;
            }
            case XR_TYPE_EXTENSION_PROPERTIES:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrExtensionProperties>());
                break;
            }
            case XR_TYPE_INSTANCE_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInstanceCreateInfo>());
                break;
            }
            case XR_TYPE_INSTANCE_PROPERTIES:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInstanceProperties>());
                break;
            }
            case XR_TYPE_EVENT_DATA_BUFFER:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataBuffer>());
                break;
            }
            case XR_TYPE_SYSTEM_GET_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemGetInfo>());
                break;
            }
            case XR_TYPE_SYSTEM_PROPERTIES:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemProperties>());
                break;
            }
            case XR_TYPE_SESSION_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSessionCreateInfo>());
                break;
            }
            case XR_TYPE_SPACE_VELOCITY:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceVelocity>());
                break;
            }
            case XR_TYPE_REFERENCE_SPACE_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrReferenceSpaceCreateInfo>());
                break;
            }
            case XR_TYPE_ACTION_SPACE_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionSpaceCreateInfo>());
                break;
            }
            case XR_TYPE_SPACE_LOCATION:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceLocation>());
                break;
            }
            case XR_TYPE_VIEW_CONFIGURATION_PROPERTIES:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewConfigurationProperties>());
                break;
            }
            case XR_TYPE_VIEW_CONFIGURATION_VIEW:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewConfigurationView>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainCreateInfo>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageAcquireInfo>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageWaitInfo>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageReleaseInfo>());
                break;
            }
            case XR_TYPE_SESSION_BEGIN_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSessionBeginInfo>());
                break;
            }
            case XR_TYPE_FRAME_WAIT_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFrameWaitInfo>());
                break;
            }
            case XR_TYPE_FRAME_STATE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFrameState>());
                break;
            }
            case XR_TYPE_FRAME_BEGIN_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFrameBeginInfo>());
                break;
            }
            case XR_TYPE_FRAME_END_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFrameEndInfo>());
                break;
            }
            case XR_TYPE_VIEW_LOCATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewLocateInfo>());
                break;
            }
            case XR_TYPE_VIEW_STATE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewState>());
                break;
            }
            case XR_TYPE_VIEW:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrView>());
                break;
            }
            case XR_TYPE_ACTION_SET_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionSetCreateInfo>());
                break;
            }
            case XR_TYPE_ACTION_CREATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionCreateInfo>());
                break;
            }
            case XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInteractionProfileSuggestedBinding>());
                break;
            }
            case XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSessionActionSetsAttachInfo>());
                break;
            }
            case XR_TYPE_INTERACTION_PROFILE_STATE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInteractionProfileState>());
                break;
            }
            case XR_TYPE_ACTION_STATE_GET_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionStateGetInfo>());
                break;
            }
            case XR_TYPE_ACTION_STATE_BOOLEAN:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionStateBoolean>());
                break;
            }
            case XR_TYPE_ACTION_STATE_FLOAT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionStateFloat>());
                break;
            }
            case XR_TYPE_ACTION_STATE_VECTOR2F:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionStateVector2f>());
                break;
            }
            case XR_TYPE_ACTION_STATE_POSE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionStatePose>());
                break;
            }
            case XR_TYPE_ACTIONS_SYNC_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActionsSyncInfo>());
                break;
            }
            case XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBoundSourcesForActionEnumerateInfo>());
                break;
            }
            case XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInputSourceLocalizedNameGetInfo>());
                break;
            }
            case XR_TYPE_HAPTIC_ACTION_INFO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHapticActionInfo>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerProjectionView>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_PROJECTION:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerProjection>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_QUAD:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerQuad>());
                break;
            }
            case XR_TYPE_EVENT_DATA_EVENTS_LOST:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataEventsLost>());
                break;
            }
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataInstanceLossPending>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSessionStateChanged>());
                break;
            }
            case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataReferenceSpaceChangePending>());
                break;
            }
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataInteractionProfileChanged>());
                break;
            }
            case XR_TYPE_HAPTIC_VIBRATION:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHapticVibration>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_CUBE_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerCubeKHR>());
                break;
            }
            case XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInstanceCreateInfoAndroidKHR>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerDepthInfoKHR>());
                break;
            }
            case XR_TYPE_VULKAN_SWAPCHAIN_FORMAT_LIST_CREATE_INFO_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVulkanSwapchainFormatListCreateInfoKHR>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerCylinderKHR>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_EQUIRECT_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerEquirectKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingOpenGLWin32KHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingOpenGLXlibKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingOpenGLXcbKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingOpenGLWaylandKHR>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageOpenGLKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsRequirementsOpenGLKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingOpenGLESAndroidKHR>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageOpenGLESKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsRequirementsOpenGLESKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingVulkanKHR>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageVulkanKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsRequirementsVulkanKHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_D3D11_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingD3D11KHR>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageD3D11KHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsRequirementsD3D11KHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_D3D12_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingD3D12KHR>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageD3D12KHR>());
                break;
            }
            case XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsRequirementsD3D12KHR>());
                break;
            }
            case XR_TYPE_VISIBILITY_MASK_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVisibilityMaskKHR>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVisibilityMaskChangedKHR>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_COLOR_SCALE_BIAS_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerColorScaleBiasKHR>());
                break;
            }
            case XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrLoaderInitInfoAndroidKHR>());
                break;
            }
            case XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVulkanInstanceCreateInfoKHR>());
                break;
            }
            case XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVulkanDeviceCreateInfoKHR>());
                break;
            }
            case XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVulkanGraphicsDeviceGetInfoKHR>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_EQUIRECT2_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerEquirect2KHR>());
                break;
            }
            case XR_TYPE_BINDING_MODIFICATIONS_KHR:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBindingModificationsKHR>());
                break;
            }
            case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataPerfSettingsEXT>());
                break;
            }
            case XR_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDebugUtilsObjectNameInfoEXT>());
                break;
            }
            case XR_TYPE_DEBUG_UTILS_LABEL_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDebugUtilsLabelEXT>());
                break;
            }
            case XR_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDebugUtilsMessengerCallbackDataEXT>());
                break;
            }
            case XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDebugUtilsMessengerCreateInfoEXT>());
                break;
            }
            case XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemEyeGazeInteractionPropertiesEXT>());
                break;
            }
            case XR_TYPE_EYE_GAZE_SAMPLE_TIME_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEyeGazeSampleTimeEXT>());
                break;
            }
            case XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSessionCreateInfoOverlayEXTX>());
                break;
            }
            case XR_TYPE_EVENT_DATA_MAIN_SESSION_VISIBILITY_CHANGED_EXTX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataMainSessionVisibilityChangedEXTX>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorSpaceCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_IMAGE_LAYOUT_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerImageLayoutFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_ALPHA_BLEND_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerAlphaBlendFB>());
                break;
            }
            case XR_TYPE_VIEW_CONFIGURATION_DEPTH_RANGE_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewConfigurationDepthRangeEXT>());
                break;
            }
            case XR_TYPE_GRAPHICS_BINDING_EGL_MNDX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGraphicsBindingEGLMNDX>());
                break;
            }
            case XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialGraphNodeSpaceCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_SPATIAL_GRAPH_STATIC_NODE_BINDING_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialGraphStaticNodeBindingCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_SPATIAL_GRAPH_NODE_BINDING_PROPERTIES_GET_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialGraphNodeBindingPropertiesGetInfoMSFT>());
                break;
            }
            case XR_TYPE_SPATIAL_GRAPH_NODE_BINDING_PROPERTIES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialGraphNodeBindingPropertiesMSFT>());
                break;
            }
            case XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemHandTrackingPropertiesEXT>());
                break;
            }
            case XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackerCreateInfoEXT>());
                break;
            }
            case XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandJointsLocateInfoEXT>());
                break;
            }
            case XR_TYPE_HAND_JOINT_LOCATIONS_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandJointLocationsEXT>());
                break;
            }
            case XR_TYPE_HAND_JOINT_VELOCITIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandJointVelocitiesEXT>());
                break;
            }
            case XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemHandTrackingMeshPropertiesMSFT>());
                break;
            }
            case XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandMeshSpaceCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandMeshUpdateInfoMSFT>());
                break;
            }
            case XR_TYPE_HAND_MESH_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandMeshMSFT>());
                break;
            }
            case XR_TYPE_HAND_POSE_TYPE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandPoseTypeInfoMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationSessionBeginInfoMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationStateMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationFrameStateMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationLayerInfoMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationFrameEndInfoMSFT>());
                break;
            }
            case XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSecondaryViewConfigurationSwapchainCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_CONTROLLER_MODEL_KEY_STATE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrControllerModelKeyStateMSFT>());
                break;
            }
            case XR_TYPE_CONTROLLER_MODEL_NODE_PROPERTIES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrControllerModelNodePropertiesMSFT>());
                break;
            }
            case XR_TYPE_CONTROLLER_MODEL_PROPERTIES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrControllerModelPropertiesMSFT>());
                break;
            }
            case XR_TYPE_CONTROLLER_MODEL_NODE_STATE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrControllerModelNodeStateMSFT>());
                break;
            }
            case XR_TYPE_CONTROLLER_MODEL_STATE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrControllerModelStateMSFT>());
                break;
            }
            case XR_TYPE_VIEW_CONFIGURATION_VIEW_FOV_EPIC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewConfigurationViewFovEPIC>());
                break;
            }
            case XR_TYPE_HOLOGRAPHIC_WINDOW_ATTACHMENT_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHolographicWindowAttachmentMSFT>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_REPROJECTION_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerReprojectionInfoMSFT>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_REPROJECTION_PLANE_OVERRIDE_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerReprojectionPlaneOverrideMSFT>());
                break;
            }
            case XR_TYPE_ANDROID_SURFACE_SWAPCHAIN_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrAndroidSurfaceSwapchainCreateInfoFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_SECURE_CONTENT_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerSecureContentFB>());
                break;
            }
            case XR_TYPE_SYSTEM_BODY_TRACKING_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemBodyTrackingPropertiesFB>());
                break;
            }
            case XR_TYPE_BODY_TRACKER_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBodyTrackerCreateInfoFB>());
                break;
            }
            case XR_TYPE_BODY_SKELETON_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBodySkeletonFB>());
                break;
            }
            case XR_TYPE_BODY_JOINTS_LOCATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBodyJointsLocateInfoFB>());
                break;
            }
            case XR_TYPE_BODY_JOINT_LOCATIONS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBodyJointLocationsFB>());
                break;
            }
            case XR_TYPE_INTERACTION_PROFILE_DPAD_BINDING_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInteractionProfileDpadBindingEXT>());
                break;
            }
            case XR_TYPE_INTERACTION_PROFILE_ANALOG_THRESHOLD_VALVE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrInteractionProfileAnalogThresholdVALVE>());
                break;
            }
            case XR_TYPE_HAND_JOINTS_MOTION_RANGE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandJointsMotionRangeInfoEXT>());
                break;
            }
            case XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneObserverCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrNewSceneComputeInfoMSFT>());
                break;
            }
            case XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVisualMeshComputeLodInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_COMPONENTS_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneComponentsMSFT>());
                break;
            }
            case XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneComponentsGetInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneComponentLocationsMSFT>());
                break;
            }
            case XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneComponentsLocateInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_OBJECTS_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneObjectsMSFT>());
                break;
            }
            case XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneComponentParentFilterInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneObjectTypesFilterInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_PLANES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrScenePlanesMSFT>());
                break;
            }
            case XR_TYPE_SCENE_PLANE_ALIGNMENT_FILTER_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrScenePlaneAlignmentFilterInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESHES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshesMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshBuffersGetInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESH_BUFFERS_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshBuffersMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshVertexBufferMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshIndicesUint32MSFT>());
                break;
            }
            case XR_TYPE_SCENE_MESH_INDICES_UINT16_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMeshIndicesUint16MSFT>());
                break;
            }
            case XR_TYPE_SERIALIZED_SCENE_FRAGMENT_DATA_GET_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSerializedSceneFragmentDataGetInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_DESERIALIZE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneDeserializeInfoMSFT>());
                break;
            }
            case XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataDisplayRefreshRateChangedFB>());
                break;
            }
            case XR_TYPE_VIVE_TRACKER_PATHS_HTCX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViveTrackerPathsHTCX>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIVE_TRACKER_CONNECTED_HTCX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataViveTrackerConnectedHTCX>());
                break;
            }
            case XR_TYPE_SYSTEM_FACIAL_TRACKING_PROPERTIES_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemFacialTrackingPropertiesHTC>());
                break;
            }
            case XR_TYPE_FACIAL_EXPRESSIONS_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFacialExpressionsHTC>());
                break;
            }
            case XR_TYPE_FACIAL_TRACKER_CREATE_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFacialTrackerCreateInfoHTC>());
                break;
            }
            case XR_TYPE_SYSTEM_COLOR_SPACE_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemColorSpacePropertiesFB>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_MESH_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingMeshFB>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_SCALE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingScaleFB>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_AIM_STATE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingAimStateFB>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_CAPSULES_STATE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingCapsulesStateFB>());
                break;
            }
            case XR_TYPE_SYSTEM_SPATIAL_ENTITY_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemSpatialEntityPropertiesFB>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorCreateInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceComponentStatusSetInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_COMPONENT_STATUS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceComponentStatusFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpatialAnchorCreateCompleteFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceSetStatusCompleteFB>());
                break;
            }
            case XR_TYPE_FOVEATION_PROFILE_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationProfileCreateInfoFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_CREATE_INFO_FOVEATION_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainCreateInfoFoveationFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_STATE_FOVEATION_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainStateFoveationFB>());
                break;
            }
            case XR_TYPE_FOVEATION_LEVEL_PROFILE_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationLevelProfileCreateInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_KEYBOARD_TRACKING_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemKeyboardTrackingPropertiesFB>());
                break;
            }
            case XR_TYPE_KEYBOARD_SPACE_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrKeyboardSpaceCreateInfoFB>());
                break;
            }
            case XR_TYPE_KEYBOARD_TRACKING_QUERY_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrKeyboardTrackingQueryFB>());
                break;
            }
            case XR_TYPE_TRIANGLE_MESH_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrTriangleMeshCreateInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemPassthroughPropertiesFB>());
                break;
            }
            case XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES2_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemPassthroughProperties2FB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughCreateInfoFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughLayerCreateInfoFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerPassthroughFB>());
                break;
            }
            case XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGeometryInstanceCreateInfoFB>());
                break;
            }
            case XR_TYPE_GEOMETRY_INSTANCE_TRANSFORM_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGeometryInstanceTransformFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_STYLE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughStyleFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_RGBA_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorMapMonoToRgbaFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_MONO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorMapMonoToMonoFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_BRIGHTNESS_CONTRAST_SATURATION_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughBrightnessContrastSaturationFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_PASSTHROUGH_STATE_CHANGED_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataPassthroughStateChangedFB>());
                break;
            }
            case XR_TYPE_RENDER_MODEL_PATH_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRenderModelPathInfoFB>());
                break;
            }
            case XR_TYPE_RENDER_MODEL_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRenderModelPropertiesFB>());
                break;
            }
            case XR_TYPE_RENDER_MODEL_BUFFER_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRenderModelBufferFB>());
                break;
            }
            case XR_TYPE_RENDER_MODEL_LOAD_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRenderModelLoadInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_RENDER_MODEL_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemRenderModelPropertiesFB>());
                break;
            }
            case XR_TYPE_RENDER_MODEL_CAPABILITIES_REQUEST_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRenderModelCapabilitiesRequestFB>());
                break;
            }
            case XR_TYPE_VIEW_LOCATE_FOVEATED_RENDERING_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrViewLocateFoveatedRenderingVARJO>());
                break;
            }
            case XR_TYPE_FOVEATED_VIEW_CONFIGURATION_VIEW_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveatedViewConfigurationViewVARJO>());
                break;
            }
            case XR_TYPE_SYSTEM_FOVEATED_RENDERING_PROPERTIES_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemFoveatedRenderingPropertiesVARJO>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerDepthTestVARJO>());
                break;
            }
            case XR_TYPE_SYSTEM_MARKER_TRACKING_PROPERTIES_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemMarkerTrackingPropertiesVARJO>());
                break;
            }
            case XR_TYPE_EVENT_DATA_MARKER_TRACKING_UPDATE_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataMarkerTrackingUpdateVARJO>());
                break;
            }
            case XR_TYPE_MARKER_SPACE_CREATE_INFO_VARJO:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerSpaceCreateInfoVARJO>());
                break;
            }
            case XR_TYPE_FRAME_END_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFrameEndInfoML>());
                break;
            }
            case XR_TYPE_GLOBAL_DIMMER_FRAME_END_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrGlobalDimmerFrameEndInfoML>());
                break;
            }
            case XR_TYPE_COORDINATE_SPACE_CREATE_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCoordinateSpaceCreateInfoML>());
                break;
            }
            case XR_TYPE_SYSTEM_MARKER_UNDERSTANDING_PROPERTIES_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemMarkerUnderstandingPropertiesML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_CREATE_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorCreateInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_ARUCO_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorArucoInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_SIZE_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorSizeInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_APRIL_TAG_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorAprilTagInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_CUSTOM_PROFILE_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorCustomProfileInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_SNAPSHOT_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorSnapshotInfoML>());
                break;
            }
            case XR_TYPE_MARKER_DETECTOR_STATE_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerDetectorStateML>());
                break;
            }
            case XR_TYPE_MARKER_SPACE_CREATE_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMarkerSpaceCreateInfoML>());
                break;
            }
            case XR_TYPE_LOCALIZATION_MAP_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrLocalizationMapML>());
                break;
            }
            case XR_TYPE_EVENT_DATA_LOCALIZATION_CHANGED_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataLocalizationChangedML>());
                break;
            }
            case XR_TYPE_MAP_LOCALIZATION_REQUEST_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrMapLocalizationRequestInfoML>());
                break;
            }
            case XR_TYPE_LOCALIZATION_MAP_IMPORT_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrLocalizationMapImportInfoML>());
                break;
            }
            case XR_TYPE_LOCALIZATION_ENABLE_EVENTS_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrLocalizationEnableEventsInfoML>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_PERSISTENCE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorPersistenceInfoMSFT>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_FROM_PERSISTED_ANCHOR_CREATE_INFO_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorFromPersistedAnchorCreateInfoMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MARKERS_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMarkersMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMarkerTypeFilterMSFT>());
                break;
            }
            case XR_TYPE_SCENE_MARKER_QR_CODES_MSFT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneMarkerQRCodesMSFT>());
                break;
            }
            case XR_TYPE_SPACE_QUERY_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceQueryInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceStorageLocationFilterInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_UUID_FILTER_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceUuidFilterInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_COMPONENT_FILTER_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceComponentFilterInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_QUERY_RESULTS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceQueryResultsFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceQueryResultsAvailableFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceQueryCompleteFB>());
                break;
            }
            case XR_TYPE_SPACE_SAVE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceSaveInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_ERASE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceEraseInfoFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceSaveCompleteFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceEraseCompleteFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_IMAGE_FOVEATION_VULKAN_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainImageFoveationVulkanFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_STATE_ANDROID_SURFACE_DIMENSIONS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainStateAndroidSurfaceDimensionsFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_STATE_SAMPLER_OPENGL_ES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainStateSamplerOpenGLESFB>());
                break;
            }
            case XR_TYPE_SWAPCHAIN_STATE_SAMPLER_VULKAN_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSwapchainStateSamplerVulkanFB>());
                break;
            }
            case XR_TYPE_SPACE_SHARE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceShareInfoFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_SHARE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceShareCompleteFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_SPACE_WARP_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerSpaceWarpInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_SPACE_WARP_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemSpaceWarpPropertiesFB>());
                break;
            }
            case XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHapticAmplitudeEnvelopeVibrationFB>());
                break;
            }
            case XR_TYPE_SEMANTIC_LABELS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSemanticLabelsFB>());
                break;
            }
            case XR_TYPE_ROOM_LAYOUT_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRoomLayoutFB>());
                break;
            }
            case XR_TYPE_BOUNDARY_2D_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrBoundary2DFB>());
                break;
            }
            case XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSemanticLabelsSupportInfoFB>());
                break;
            }
            case XR_TYPE_DIGITAL_LENS_CONTROL_ALMALENCE:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDigitalLensControlALMALENCE>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSceneCaptureCompleteFB>());
                break;
            }
            case XR_TYPE_SCENE_CAPTURE_REQUEST_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSceneCaptureRequestInfoFB>());
                break;
            }
            case XR_TYPE_SPACE_CONTAINER_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceContainerFB>());
                break;
            }
            case XR_TYPE_FOVEATION_EYE_TRACKED_PROFILE_CREATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationEyeTrackedProfileCreateInfoMETA>());
                break;
            }
            case XR_TYPE_FOVEATION_EYE_TRACKED_STATE_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationEyeTrackedStateMETA>());
                break;
            }
            case XR_TYPE_SYSTEM_FOVEATION_EYE_TRACKED_PROPERTIES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemFoveationEyeTrackedPropertiesMETA>());
                break;
            }
            case XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemFaceTrackingPropertiesFB>());
                break;
            }
            case XR_TYPE_FACE_TRACKER_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceTrackerCreateInfoFB>());
                break;
            }
            case XR_TYPE_FACE_EXPRESSION_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceExpressionInfoFB>());
                break;
            }
            case XR_TYPE_FACE_EXPRESSION_WEIGHTS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceExpressionWeightsFB>());
                break;
            }
            case XR_TYPE_EYE_TRACKER_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEyeTrackerCreateInfoFB>());
                break;
            }
            case XR_TYPE_EYE_GAZES_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEyeGazesInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_EYE_TRACKING_PROPERTIES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemEyeTrackingPropertiesFB>());
                break;
            }
            case XR_TYPE_EYE_GAZES_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEyeGazesFB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_KEYBOARD_HANDS_INTENSITY_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughKeyboardHandsIntensityFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_SETTINGS_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerSettingsFB>());
                break;
            }
            case XR_TYPE_HAPTIC_PCM_VIBRATION_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHapticPcmVibrationFB>());
                break;
            }
            case XR_TYPE_DEVICE_PCM_SAMPLE_RATE_STATE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrDevicePcmSampleRateStateFB>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerDepthTestFB>());
                break;
            }
            case XR_TYPE_LOCAL_DIMMING_FRAME_END_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrLocalDimmingFrameEndInfoMETA>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_PREFERENCES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughPreferencesMETA>());
                break;
            }
            case XR_TYPE_SYSTEM_VIRTUAL_KEYBOARD_PROPERTIES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemVirtualKeyboardPropertiesMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_CREATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardCreateInfoMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_SPACE_CREATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardSpaceCreateInfoMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_LOCATION_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardLocationInfoMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_MODEL_VISIBILITY_SET_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardModelVisibilitySetInfoMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_ANIMATION_STATE_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardAnimationStateMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_MODEL_ANIMATION_STATES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardModelAnimationStatesMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_TEXTURE_DATA_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardTextureDataMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_INPUT_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardInputInfoMETA>());
                break;
            }
            case XR_TYPE_VIRTUAL_KEYBOARD_TEXT_CONTEXT_CHANGE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVirtualKeyboardTextContextChangeInfoMETA>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_COMMIT_TEXT_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVirtualKeyboardCommitTextMETA>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_BACKSPACE_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVirtualKeyboardBackspaceMETA>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_ENTER_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVirtualKeyboardEnterMETA>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_SHOWN_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVirtualKeyboardShownMETA>());
                break;
            }
            case XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_HIDDEN_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataVirtualKeyboardHiddenMETA>());
                break;
            }
            case XR_TYPE_EXTERNAL_CAMERA_OCULUS:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrExternalCameraOCULUS>());
                break;
            }
            case XR_TYPE_VULKAN_SWAPCHAIN_CREATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrVulkanSwapchainCreateInfoMETA>());
                break;
            }
            case XR_TYPE_PERFORMANCE_METRICS_STATE_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPerformanceMetricsStateMETA>());
                break;
            }
            case XR_TYPE_PERFORMANCE_METRICS_COUNTER_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPerformanceMetricsCounterMETA>());
                break;
            }
            case XR_TYPE_SPACE_LIST_SAVE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceListSaveInfoFB>());
                break;
            }
            case XR_TYPE_EVENT_DATA_SPACE_LIST_SAVE_COMPLETE_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataSpaceListSaveCompleteFB>());
                break;
            }
            case XR_TYPE_SPACE_USER_CREATE_INFO_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceUserCreateInfoFB>());
                break;
            }
            case XR_TYPE_SYSTEM_HEADSET_ID_PROPERTIES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemHeadsetIdPropertiesMETA>());
                break;
            }
            case XR_TYPE_RECOMMENDED_LAYER_RESOLUTION_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRecommendedLayerResolutionMETA>());
                break;
            }
            case XR_TYPE_RECOMMENDED_LAYER_RESOLUTION_GET_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrRecommendedLayerResolutionGetInfoMETA>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_LUT_CREATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorLutCreateInfoMETA>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_LUT_UPDATE_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorLutUpdateInfoMETA>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_MAP_LUT_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorMapLutMETA>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_MAP_INTERPOLATED_LUT_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorMapInterpolatedLutMETA>());
                break;
            }
            case XR_TYPE_SYSTEM_PASSTHROUGH_COLOR_LUT_PROPERTIES_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemPassthroughColorLutPropertiesMETA>());
                break;
            }
            case XR_TYPE_SPACE_TRIANGLE_MESH_GET_INFO_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceTriangleMeshGetInfoMETA>());
                break;
            }
            case XR_TYPE_SPACE_TRIANGLE_MESH_META:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpaceTriangleMeshMETA>());
                break;
            }
            case XR_TYPE_SYSTEM_FACE_TRACKING_PROPERTIES2_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemFaceTrackingProperties2FB>());
                break;
            }
            case XR_TYPE_FACE_TRACKER_CREATE_INFO2_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceTrackerCreateInfo2FB>());
                break;
            }
            case XR_TYPE_FACE_EXPRESSION_INFO2_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceExpressionInfo2FB>());
                break;
            }
            case XR_TYPE_FACE_EXPRESSION_WEIGHTS2_FB:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFaceExpressionWeights2FB>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_CREATE_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughCreateInfoHTC>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_COLOR_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughColorHTC>());
                break;
            }
            case XR_TYPE_PASSTHROUGH_MESH_TRANSFORM_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPassthroughMeshTransformInfoHTC>());
                break;
            }
            case XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrCompositionLayerPassthroughHTC>());
                break;
            }
            case XR_TYPE_FOVEATION_APPLY_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationApplyInfoHTC>());
                break;
            }
            case XR_TYPE_FOVEATION_DYNAMIC_MODE_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationDynamicModeInfoHTC>());
                break;
            }
            case XR_TYPE_FOVEATION_CUSTOM_MODE_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrFoveationCustomModeInfoHTC>());
                break;
            }
            case XR_TYPE_SYSTEM_ANCHOR_PROPERTIES_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemAnchorPropertiesHTC>());
                break;
            }
            case XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_HTC:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSpatialAnchorCreateInfoHTC>());
                break;
            }
            case XR_TYPE_ACTIVE_ACTION_SET_PRIORITIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrActiveActionSetPrioritiesEXT>());
                break;
            }
            case XR_TYPE_SYSTEM_FORCE_FEEDBACK_CURL_PROPERTIES_MNDX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemForceFeedbackCurlPropertiesMNDX>());
                break;
            }
            case XR_TYPE_FORCE_FEEDBACK_CURL_APPLY_LOCATIONS_MNDX:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrForceFeedbackCurlApplyLocationsMNDX>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_DATA_SOURCE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingDataSourceInfoEXT>());
                break;
            }
            case XR_TYPE_HAND_TRACKING_DATA_SOURCE_STATE_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrHandTrackingDataSourceStateEXT>());
                break;
            }
            case XR_TYPE_SYSTEM_PLANE_DETECTION_PROPERTIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemPlaneDetectionPropertiesEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_CREATE_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorCreateInfoEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_BEGIN_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorBeginInfoEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_GET_INFO_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorGetInfoEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_LOCATION_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorLocationEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_LOCATIONS_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorLocationsEXT>());
                break;
            }
            case XR_TYPE_PLANE_DETECTOR_POLYGON_BUFFER_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrPlaneDetectorPolygonBufferEXT>());
                break;
            }
            case XR_TYPE_EVENT_DATA_USER_PRESENCE_CHANGED_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataUserPresenceChangedEXT>());
                break;
            }
            case XR_TYPE_SYSTEM_USER_PRESENCE_PROPERTIES_EXT:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrSystemUserPresencePropertiesEXT>());
                break;
            }
            case XR_TYPE_EVENT_DATA_HEADSET_FIT_CHANGED_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataHeadsetFitChangedML>());
                break;
            }
            case XR_TYPE_EVENT_DATA_EYE_CALIBRATION_CHANGED_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrEventDataEyeCalibrationChangedML>());
                break;
            }
            case XR_TYPE_USER_CALIBRATION_ENABLE_EVENTS_INFO_ML:
            {
                output_struct->next = reinterpret_cast<XrBaseOutStructure*>(DecodeAllocator::Allocate<XrUserCalibrationEnableEventsInfoML>());
                break;
            }
            default:
                break;
        }
        output_struct = output_struct->next;
        output_struct->type = in_next->type;
        in_next = in_next->next;
    }
}

template <typename T>
void InitializeOutputStructNext(StructPointerDecoder<T> *decoder)
{
    if(decoder->IsNull()) return;
    size_t len = decoder->GetOutputLength();
    auto input = decoder->GetPointer();
    auto output = decoder->GetOutputPointer();
    for( size_t i = 0 ; i < len; ++i )
    {
        const auto* in_next = reinterpret_cast<const XrBaseInStructure*>(input[i].next);
        if( in_next == nullptr ) continue;
        auto* output_struct = reinterpret_cast<XrBaseOutStructure*>(&output[i]);
        InitializeOutputStructNextImpl(in_next, output_struct);
    }
}

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // ENABLE_OPENXR_SUPPORT

#endif //  GFXRECON_GENERATED_OPENXR_STRUCT_INIT_NEXT_CHAIN_H
