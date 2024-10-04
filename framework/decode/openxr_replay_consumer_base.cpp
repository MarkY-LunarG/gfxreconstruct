/*
** Copyright (c) 2024 LunarG, Inc.
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

#if ENABLE_OPENXR_SUPPORT
#include <array>

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
#endif

#include "util/platform.h"
#include "util/defines.h"

#ifdef XR_NO_PROTOTYPES
#undef XR_NO_PROTOTYPES
#endif
#define XR_EXTENSION_PROTOTYPES

#include "format/platform_types.h"

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

#include "decode/openxr_feature_util.h"
#include "decode/openxr_handle_mapping_util.h"
#include "decode/vulkan_handle_mapping_util.h"

#include "openxr_replay_consumer_base.h"
#include "vulkan_replay_consumer_base.h"

#include "generated/generated_openxr_enum_to_string.h"
#include "generated/generated_openxr_struct_handle_mappers.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

OpenXrReplayConsumerBase::OpenXrReplayConsumerBase(std::shared_ptr<application::Application> application,
                                                   const OpenXrReplayOptions&                options) :
    application_(application),
    options_(options), get_instance_proc_addr_(nullptr)
{
    assert(application_ != nullptr);
    object_info_table_ = CommonObjectInfoTable::GetSingleton();
}

OpenXrReplayConsumerBase::~OpenXrReplayConsumerBase() {}

void OpenXrReplayConsumerBase::SetVulkanReplayConsumer(VulkanReplayConsumerBase* vulkan_replay_consumer)
{
    vulkan_replay_consumer_ = vulkan_replay_consumer;
}

void OpenXrReplayConsumerBase::AddInstanceTable(XrInstance instance)
{
    encode::OpenXrInstanceTable& table = instance_tables_[instance];
    encode::LoadOpenXrInstanceTable(get_instance_proc_addr_, instance, &table);
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrInstance handle) const
{
    auto table = instance_tables_.find(handle);
    assert(table != instance_tables_.end());
    return (table != instance_tables_.end()) ? &table->second : nullptr;
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
// NOTE: This won't work on 32-bit builds because OpenXR defines all 32-bit
//       handles as uint64_t breaking the type conversion on each of these
//       override functions.

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSession handle) const
{
    auto instance = session_to_instance_.find(handle);
    assert(instance != session_to_instance_.end());
    return (instance != session_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSpace handle) const
{
    auto instance = space_to_instance_.find(handle);
    assert(instance != space_to_instance_.end());
    return (instance != space_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSwapchain handle) const
{
    auto instance = swapchain_to_instance_.find(handle);
    assert(instance != swapchain_to_instance_.end());
    return (instance != swapchain_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrActionSet handle) const
{
    auto instance = actionset_to_instance_.find(handle);
    assert(instance != actionset_to_instance_.end());
    return (instance != actionset_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrAction handle) const
{
    auto instance = action_to_instance_.find(handle);
    assert(instance != action_to_instance_.end());
    return (instance != action_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrDebugUtilsMessengerEXT handle) const
{
    auto instance = debugutilsmessengerEXT_to_instance_.find(handle);
    assert(instance != debugutilsmessengerEXT_to_instance_.end());
    return (instance != debugutilsmessengerEXT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSpatialAnchorMSFT handle) const
{
    auto instance = spatialanchorMSFT_to_instance_.find(handle);
    assert(instance != spatialanchorMSFT_to_instance_.end());
    return (instance != spatialanchorMSFT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrHandTrackerEXT handle) const
{
    auto instance = handtrackerEXT_to_instance_.find(handle);
    assert(instance != handtrackerEXT_to_instance_.end());
    return (instance != handtrackerEXT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrBodyTrackerFB handle) const
{
    auto instance = bodytrackerFB_to_instance_.find(handle);
    assert(instance != bodytrackerFB_to_instance_.end());
    return (instance != bodytrackerFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSceneObserverMSFT handle) const
{
    auto instance = sceneobserverMSFT_to_instance_.find(handle);
    assert(instance != sceneobserverMSFT_to_instance_.end());
    return (instance != sceneobserverMSFT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSceneMSFT handle) const
{
    auto instance = sceneMSFT_to_instance_.find(handle);
    assert(instance != sceneMSFT_to_instance_.end());
    return (instance != sceneMSFT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrFacialTrackerHTC handle) const
{
    auto instance = facialtrackerHTC_to_instance_.find(handle);
    assert(instance != facialtrackerHTC_to_instance_.end());
    return (instance != facialtrackerHTC_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrFoveationProfileFB handle) const
{
    auto instance = foveationprofileFB_to_instance_.find(handle);
    assert(instance != foveationprofileFB_to_instance_.end());
    return (instance != foveationprofileFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrPassthroughFB handle) const
{
    auto instance = passthroughFB_to_instance_.find(handle);
    assert(instance != passthroughFB_to_instance_.end());
    return (instance != passthroughFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrPassthroughLayerFB handle) const
{
    auto instance = passthroughlayerFB_to_instance_.find(handle);
    assert(instance != passthroughlayerFB_to_instance_.end());
    return (instance != passthroughlayerFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrGeometryInstanceFB handle) const
{
    auto instance = geometryinstanceFB_to_instance_.find(handle);
    assert(instance != geometryinstanceFB_to_instance_.end());
    return (instance != geometryinstanceFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrPlaneDetectorEXT handle) const
{
    auto instance = planedetectorEXT_to_instance_.find(handle);
    assert(instance != planedetectorEXT_to_instance_.end());
    return (instance != planedetectorEXT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrPassthroughHTC handle) const
{
    auto instance = passthroughHTC_to_instance_.find(handle);
    assert(instance != passthroughHTC_to_instance_.end());
    return (instance != passthroughHTC_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrMarkerDetectorML handle) const
{
    auto instance = markerdetectorML_to_instance_.find(handle);
    assert(instance != markerdetectorML_to_instance_.end());
    return (instance != markerdetectorML_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrExportedLocalizationMapML handle) const
{
    auto instance = exportedlocalicationML_to_instance_.find(handle);
    assert(instance != exportedlocalicationML_to_instance_.end());
    return (instance != exportedlocalicationML_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable*
OpenXrReplayConsumerBase::GetInstanceTable(XrSpatialAnchorStoreConnectionMSFT handle) const
{
    auto instance = spatialanchorconnectionMSFT_to_instance_.find(handle);
    assert(instance != spatialanchorconnectionMSFT_to_instance_.end());
    return (instance != spatialanchorconnectionMSFT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrFaceTrackerFB handle) const
{
    auto instance = facetrackerFB_to_instance_.find(handle);
    assert(instance != facetrackerFB_to_instance_.end());
    return (instance != facetrackerFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrEyeTrackerFB handle) const
{
    auto instance = eyetrackerFB_to_instance_.find(handle);
    assert(instance != eyetrackerFB_to_instance_.end());
    return (instance != eyetrackerFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrVirtualKeyboardMETA handle) const
{
    auto instance = virtualkeyboardMETA_to_instance_.find(handle);
    assert(instance != virtualkeyboardMETA_to_instance_.end());
    return (instance != virtualkeyboardMETA_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrSpaceUserFB handle) const
{
    auto instance = spaceuserFB_to_instance_.find(handle);
    assert(instance != spaceuserFB_to_instance_.end());
    return (instance != spaceuserFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrPassthroughColorLutMETA handle) const
{
    auto instance = passthroughcolorlutMETA_to_instance_.find(handle);
    assert(instance != passthroughcolorlutMETA_to_instance_.end());
    return (instance != passthroughcolorlutMETA_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrFaceTracker2FB handle) const
{
    auto instance = facetracker2FB_to_instance_.find(handle);
    assert(instance != facetracker2FB_to_instance_.end());
    return (instance != facetracker2FB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable*
OpenXrReplayConsumerBase::GetInstanceTable(XrSpatialGraphNodeBindingMSFT handle) const
{
    auto instance = spatialgraphnodebindingMSFT_to_instance_.find(handle);
    assert(instance != spatialgraphnodebindingMSFT_to_instance_.end());
    return (instance != spatialgraphnodebindingMSFT_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable* OpenXrReplayConsumerBase::GetInstanceTable(XrTriangleMeshFB handle) const
{
    auto instance = trianglemeshFB_to_instance_.find(handle);
    assert(instance != trianglemeshFB_to_instance_.end());
    return (instance != trianglemeshFB_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable*
OpenXrReplayConsumerBase::GetInstanceTable(XrEnvironmentDepthProviderMETA handle) const
{
    auto instance = envdepthproviderMETA_to_instance_.find(handle);
    assert(instance != envdepthproviderMETA_to_instance_.end());
    return (instance != envdepthproviderMETA_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

const encode::OpenXrInstanceTable*
OpenXrReplayConsumerBase::GetInstanceTable(XrEnvironmentDepthSwapchainMETA handle) const
{
    auto instance = envdepthswapchainMETA_to_instance_.find(handle);
    assert(instance != envdepthswapchainMETA_to_instance_.end());
    return (instance != envdepthswapchainMETA_to_instance_.end()) ? GetInstanceTable(instance->second) : nullptr;
}

#endif // defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)

void OpenXrReplayConsumerBase::AssociateParent(XrSession session, XrInstance instance)
{
    session_to_instance_[session] = instance;
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
void OpenXrReplayConsumerBase::AssociateParent(XrSpace space, XrSession session)
{
    space_to_instance_[space] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSwapchain swapchain, XrSession session)
{
    swapchain_to_instance_[swapchain] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrActionSet action_set, XrInstance instance)
{
    actionset_to_instance_[action_set] = instance;
}

void OpenXrReplayConsumerBase::AssociateParent(XrAction action, XrActionSet action_set)
{
    action_to_instance_[action] = actionset_to_instance_[action_set];
}

void OpenXrReplayConsumerBase::AssociateParent(XrDebugUtilsMessengerEXT debug_messenger, XrInstance instance)
{
    debugutilsmessengerEXT_to_instance_[debug_messenger] = instance;
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpatialAnchorMSFT spatial_anchor, XrSession session)
{
    spatialanchorMSFT_to_instance_[spatial_anchor] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrHandTrackerEXT hand_tracker, XrSession session)
{
    handtrackerEXT_to_instance_[hand_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpace space, XrHandTrackerEXT hand_tracker)
{
    space_to_instance_[space] = handtrackerEXT_to_instance_[hand_tracker];
}

void OpenXrReplayConsumerBase::AssociateParent(XrBodyTrackerFB body_tracker, XrSession session)
{
    bodytrackerFB_to_instance_[body_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSceneObserverMSFT scene_observer, XrSession session)
{
    sceneobserverMSFT_to_instance_[scene_observer] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSceneMSFT scene, XrSceneObserverMSFT scene_observer)
{
    sceneMSFT_to_instance_[scene] = sceneobserverMSFT_to_instance_[scene_observer];
}

void OpenXrReplayConsumerBase::AssociateParent(XrFacialTrackerHTC facial_tracker, XrSession session)
{
    facialtrackerHTC_to_instance_[facial_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrFoveationProfileFB foveation_profile, XrSession session)
{
    foveationprofileFB_to_instance_[foveation_profile] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrPassthroughFB passthrough, XrSession session)
{
    passthroughFB_to_instance_[passthrough] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrPassthroughLayerFB passthrough_layer, XrSession session)
{
    passthroughlayerFB_to_instance_[passthrough_layer] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrGeometryInstanceFB geometry_instance, XrSession session)
{
    geometryinstanceFB_to_instance_[geometry_instance] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrMarkerDetectorML marker_detector, XrSession session)
{
    markerdetectorML_to_instance_[marker_detector] = session_to_instance_[session];
}
void OpenXrReplayConsumerBase::AssociateParent(XrExportedLocalizationMapML exported_local_map, XrSession session)
{
    exportedlocalicationML_to_instance_[exported_local_map] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpatialAnchorStoreConnectionMSFT spatial_anchor_store,
                                               XrSession                          session)
{
    spatialanchorconnectionMSFT_to_instance_[spatial_anchor_store] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrFaceTrackerFB face_tracker, XrSession session)
{
    facetrackerFB_to_instance_[face_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrEyeTrackerFB eye_tracker, XrSession session)
{
    eyetrackerFB_to_instance_[eye_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrVirtualKeyboardMETA virtual_keyboard, XrSession session)
{
    virtualkeyboardMETA_to_instance_[virtual_keyboard] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpace space, XrVirtualKeyboardMETA virtual_keyboard)
{
    space_to_instance_[space] = virtualkeyboardMETA_to_instance_[virtual_keyboard];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpaceUserFB space_user, XrSession session)
{
    spaceuserFB_to_instance_[space_user] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrPassthroughColorLutMETA passthrough_color_lut,
                                               XrPassthroughFB           passthrough)
{
    passthroughcolorlutMETA_to_instance_[passthrough_color_lut] = passthroughFB_to_instance_[passthrough];
}

void OpenXrReplayConsumerBase::AssociateParent(XrFaceTracker2FB face_tracker, XrSession session)
{
    facetracker2FB_to_instance_[face_tracker] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrPassthroughHTC passthrough, XrSession session)
{
    passthroughHTC_to_instance_[passthrough] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrPlaneDetectorEXT plane_detector, XrSession session)
{
    planedetectorEXT_to_instance_[plane_detector] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrSpatialGraphNodeBindingMSFT graph_node, XrSession session)
{
    spatialgraphnodebindingMSFT_to_instance_[graph_node] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrTriangleMeshFB triangle_mesh, XrSession session)
{
    trianglemeshFB_to_instance_[triangle_mesh] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrEnvironmentDepthProviderMETA env_depth_provider, XrSession session)
{
    envdepthproviderMETA_to_instance_[env_depth_provider] = session_to_instance_[session];
}

void OpenXrReplayConsumerBase::AssociateParent(XrEnvironmentDepthSwapchainMETA env_depth_swapchain,
                                               XrEnvironmentDepthProviderMETA  environment_depth_provider)
{
    envdepthswapchainMETA_to_instance_[env_depth_swapchain] =
        envdepthproviderMETA_to_instance_[environment_depth_provider];
}

#endif // defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)

void OpenXrReplayConsumerBase::Process_xrInitializeLoaderKHR(
    const ApiCallInfo&                                           call_info,
    XrResult                                                     returnValue,
    StructPointerDecoder<Decoded_XrLoaderInitInfoBaseHeaderKHR>* loaderInitInfo)
{
    XrResult replay_result = XR_SUCCESS;

    PFN_xrInitializeLoaderKHR pfn_initialize_loader;
    xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&pfn_initialize_loader);
    if (pfn_initialize_loader)
    {
#if defined(__ANDROID__)
        XrLoaderInitInfoAndroidKHR loader_init = {
            XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR,
            nullptr,
            android_app_->activity->vm,
            android_app_->activity->clazz,
        };

        replay_result = pfn_initialize_loader((XrLoaderInitInfoBaseHeaderKHR*)&loader_init);
#endif
    }
    CheckResult("xrInitializeLoaderKHR", returnValue, replay_result, call_info);
}

void OpenXrReplayConsumerBase::Process_xrCreateApiLayerInstance(
    const ApiCallInfo&                                  call_info,
    XrResult                                            returnValue,
    StructPointerDecoder<Decoded_XrInstanceCreateInfo>* info,
    StructPointerDecoder<Decoded_XrApiLayerCreateInfo>* apiLayerInfo,
    HandlePointerDecoder<XrInstance>*                   instance)
{
    if (!instance->IsNull())
    {
        instance->SetHandleLength(1);
    }
    XrInstance* replay_instance = instance->GetHandlePointer();

    if (get_instance_proc_addr_ == nullptr)
    {
        get_instance_proc_addr_ = xrGetInstanceProcAddr;
    }

    XrInstanceCreateInfo* create_info = info->GetPointer();
    assert(create_info);

    std::vector<const char*> modified_extensions;
    XrInstanceCreateInfo     modified_create_info;
    memcpy(&modified_create_info, create_info, sizeof(XrInstanceCreateInfo));

    // Transfer requested extensions to filtered extension
    for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i)
    {
        modified_extensions.push_back(create_info->enabledExtensionNames[i]);
    }

    // Proc addresses that can't be used in layers so are not generated into shared dispatch table, but are needed in
    // the replay application.
    PFN_xrEnumerateInstanceExtensionProperties instance_extension_proc;
    xrGetInstanceProcAddr(
        XR_NULL_HANDLE, "xrEnumerateInstanceExtensionProperties", (PFN_xrVoidFunction*)&instance_extension_proc);

    // Sanity checks depending on extension availability
    std::vector<XrExtensionProperties> available_extensions;
    if (feature_util::GetInstanceExtensions(instance_extension_proc, &available_extensions) == XR_SUCCESS)
    {
        if (options_.remove_unsupported_features)
        {
            // Remove enabled extensions that are not available from the replay instance.
            feature_util::RemoveUnsupportedExtensions(available_extensions, &modified_extensions);
        }
    }
    else
    {
        GFXRECON_LOG_WARNING("Failed to get instance extensions. Cannot perform sanity checks or filters for "
                             "extension availability.");
    }

#if defined(__ANDROID__)
    XrInstanceCreateInfoAndroidKHR init_android = {
        XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR,
        create_info->next,
        android_app_->activity->vm,
        android_app_->activity->clazz,
    };

    // Remove the original XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR structure with incorrect info
    XrBaseInStructure*       last_next_struct = reinterpret_cast<XrBaseInStructure*>(&init_android);
    const XrBaseInStructure* next_struct      = reinterpret_cast<const XrBaseInStructure*>(create_info->next);
    while (next_struct != nullptr)
    {
        // Skip any old Android create info structs
        if (next_struct->type == XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR)
        {
            last_next_struct->next = next_struct->next;
        }
        else
        {
            last_next_struct = const_cast<XrBaseInStructure*>(next_struct);
        }
        next_struct = next_struct->next;
    }
    modified_create_info.next = &init_android;
#endif // IGL_PLATFORM_ANDROID

    modified_create_info.enabledExtensionCount = static_cast<uint32_t>(modified_extensions.size());
    modified_create_info.enabledExtensionNames = modified_extensions.data();

    auto replay_result = xrCreateInstance(&modified_create_info, replay_instance);
    CheckResult("xrCreateApiLayerInstance", returnValue, replay_result, call_info);

    AddInstanceTable(*replay_instance);

    // Create the mapping between the recorded and replay instance handles
    AddHandle<OpenXrInstanceInfo>(format::kNullHandleId,
                                  instance->GetPointer(),
                                  instance->GetHandlePointer(),
                                  &CommonObjectInfoTable::AddXrInstanceInfo);
}

void OpenXrReplayConsumerBase::Process_xrCreateVulkanInstanceKHR(
    const ApiCallInfo&                                           call_info,
    XrResult                                                     returnValue,
    format::HandleId                                             instance,
    StructPointerDecoder<Decoded_XrVulkanInstanceCreateInfoKHR>* createInfo,
    HandlePointerDecoder<VkInstance>*                            vulkanInstance,
    PointerDecoder<VkResult>*                                    vulkanResult)
{
    XrInstance in_instance = MapHandle<OpenXrInstanceInfo>(instance, &CommonObjectInfoTable::GetXrInstanceInfo);
    const XrVulkanInstanceCreateInfoKHR* in_createInfo = createInfo->GetPointer();
    MapStructHandles(createInfo->GetMetaStructPointer(), GetObjectInfoTable());
    StructPointerDecoder<Decoded_VkInstanceCreateInfo>* vulkanCreateInfo =
        createInfo->GetMetaStructPointer()->vulkanCreateInfo;

    if (!vulkanInstance->IsNull())
    {
        vulkanInstance->SetHandleLength(1);
    }
    VkInstance*        out_vulkanInstance = vulkanInstance->GetHandlePointer();
    VulkanInstanceInfo vulkan_handle_info;
    vulkanInstance->SetConsumerData(
        0, &vulkan_handle_info); // To match what the shared routines from vkCreateInstance expect

    // Customize the Vulkan instance the way Vulkan Replay does.
    VulkanReplayConsumerBase::CreateInstanceInfoState create_state;
    vulkan_replay_consumer_->ModifyCreateInstanceInfo(vulkanCreateInfo, create_state);

    // Make a shallow copy and touch-up
    XrVulkanInstanceCreateInfoKHR replay_info = *in_createInfo;
    replay_info.vulkanCreateInfo              = &create_state.modified_create_info;
    replay_info.pfnGetInstanceProcAddr        = vulkan_replay_consumer_->GetGetInstanceProcAddr();

    VkResult replay_vulkan_result = VK_RESULT_MAX_ENUM; // An invalid value
    XrResult replay_result =
        GetInstanceTable(in_instance)
            ->CreateVulkanInstanceKHR(in_instance, &replay_info, out_vulkanInstance, &replay_vulkan_result);
    CheckResult("xrCreateVulkanInstanceKHR", returnValue, replay_result, call_info);

    // We also need to check the Vulkan Result
    VkResult* in_vulkan_result = vulkanResult->GetPointer();
    assert(in_vulkan_result);
    vulkan_replay_consumer_->CheckResult(
        "xrCreateVulkanInstanceKHR", *in_vulkan_result, replay_vulkan_result, call_info);

    if (replay_vulkan_result == VK_SUCCESS)
    {
        vulkan_replay_consumer_->PostCreateInstanceUpdateState(
            *out_vulkanInstance, create_state.modified_create_info, vulkan_handle_info);
    }

    AddHandle<VulkanInstanceInfo>(instance,
                                  vulkanInstance->GetPointer(),
                                  out_vulkanInstance,
                                  std::move(vulkan_handle_info),
                                  &CommonObjectInfoTable::AddVkInstanceInfo);
}

void OpenXrReplayConsumerBase::Process_xrCreateVulkanDeviceKHR(
    const ApiCallInfo&                                         call_info,
    XrResult                                                   returnValue,
    format::HandleId                                           instance,
    StructPointerDecoder<Decoded_XrVulkanDeviceCreateInfoKHR>* createInfo,
    HandlePointerDecoder<VkDevice>*                            vulkanDevice,
    PointerDecoder<VkResult>*                                  vulkanResult)
{
    XrInstance in_instance = MapHandle<OpenXrInstanceInfo>(instance, &CommonObjectInfoTable::GetXrInstanceInfo);
    const XrVulkanDeviceCreateInfoKHR* in_createInfo = createInfo->GetPointer();
    MapStructHandles(createInfo->GetMetaStructPointer(), GetObjectInfoTable());

    Decoded_XrVulkanDeviceCreateInfoKHR* xr_create_info_wrapper = createInfo->GetMetaStructPointer();
    assert(xr_create_info_wrapper);

    VulkanPhysicalDeviceInfo* physical_device_info =
        GetObjectInfoTable().GetVkPhysicalDeviceInfo(xr_create_info_wrapper->vulkanPhysicalDevice);
    assert(
        physical_device_info); // The Vulkan replay consumer doesn't test the result or LOG this so just assert for now

    // NOTE: The "GetMatchingDevice" process has side effects that are needed, though it's unclear whether
    //       the matching process is valid for Xr
    vulkan_replay_consumer_->GetMatchingDevice(physical_device_info);

    VulkanReplayConsumerBase::CreateDeviceInfoState create_state;
    vulkan_replay_consumer_->ModifyCreateDeviceInfo(
        physical_device_info, xr_create_info_wrapper->vulkanCreateInfo, create_state);
    XrVulkanDeviceCreateInfoKHR replay_info = *in_createInfo;
    replay_info.vulkanCreateInfo            = &create_state.modified_create_info;
    replay_info.pfnGetInstanceProcAddr      = vulkan_replay_consumer_->GetGetInstanceProcAddr();

    if (!vulkanDevice->IsNull())
    {
        vulkanDevice->SetHandleLength(1);
    }
    VkDevice* out_vulkanDevice = vulkanDevice->GetHandlePointer();
    assert(out_vulkanDevice);

    VkResult replay_vulkan_result = VK_RESULT_MAX_ENUM;

    XrResult replay_result =
        GetInstanceTable(in_instance)
            ->CreateVulkanDeviceKHR(in_instance, &replay_info, out_vulkanDevice, &replay_vulkan_result);
    CheckResult("xrCreateVulkanDeviceKHR", returnValue, replay_result, call_info);

    // There's a bit more Vulkan to call to finish device creation
    VulkanDeviceInfo device_info;
    if (replay_vulkan_result == VK_SUCCESS)
    {
        replay_vulkan_result = vulkan_replay_consumer_->PostCreateDeviceUpdateState(
            physical_device_info, *out_vulkanDevice, create_state, &device_info);
    }

    // We also need to check the Vulkan Result
    VkResult* in_vulkan_result = vulkanResult->GetPointer();
    assert(in_vulkan_result);
    vulkan_replay_consumer_->CheckResult(
        "xrCreateVulkanInstanceKHR", *in_vulkan_result, replay_vulkan_result, call_info);

    // Note: we use the physical device *alias* (if present) instead of the parameter
    //       s.t. the HandleId is consistent across the call and in later use
    AddHandle<VulkanDeviceInfo>(physical_device_info->capture_id,
                                vulkanDevice->GetPointer(),
                                out_vulkanDevice,
                                std::move(device_info),
                                &CommonObjectInfoTable::AddVkDeviceInfo);
}

void OpenXrReplayConsumerBase::UpdateState_xrCreateSession(
    const ApiCallInfo&                                 call_info,
    XrResult                                           returnValue,
    format::HandleId                                   instance,
    StructPointerDecoder<Decoded_XrSessionCreateInfo>* createInfo,
    HandlePointerDecoder<XrSession>*                   session,
    XrResult                                           replay_result)
{
    Decoded_XrSessionCreateInfo* decoded_info   = createInfo->GetMetaStructPointer();
    SessionData&                 session_data   = AddSessionData(*session->GetPointer());
    session_data.AddGraphicsBinding(MakeGraphicsBinding(decoded_info));
}

void OpenXrReplayConsumerBase::UpdateState_xrEndSession(const ApiCallInfo& call_info,
                                                        XrResult           returnValue,
                                                        format::HandleId   session,
                                                        XrResult           replay_result)
{
    SessionData& session_data = GetSessionData(session);
    session_data.ClearViewRelativeProxySpaces(GetInstanceTable(session_data.GetHandle()));
}

void OpenXrReplayConsumerBase::UpdateState_xrBeginFrame(const ApiCallInfo&                              call_info,
                                                        XrResult                                        returnValue,
                                                        format::HandleId                                session,
                                                        StructPointerDecoder<Decoded_XrFrameBeginInfo>* frameBeginInfo,
                                                        XrResult                                        replay_result)
{
    SessionData& session_data = GetSessionData(session);
    session_data.ClearViewRelativeProxySpaces(GetInstanceTable(session_data.GetHandle()));
}

struct EventStrings
{
    XrStructureType type;
    char            name[128];
};
static EventStrings events_to_string[] = {
    { XR_TYPE_UNKNOWN, "Unknown Event Type" },
    { XR_TYPE_EVENT_DATA_BUFFER, "XR_TYPE_EVENT_DATA_BUFFER" },
    { XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING, "XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING" },
    { XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED, "XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED" },
    { XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING, "XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING" },
    { XR_TYPE_EVENT_DATA_EVENTS_LOST, "XR_TYPE_EVENT_DATA_EVENTS_LOST" },
    { XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED, "XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED" },
    { XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT, "XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT" },
    { XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR, "XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR" },
    { XR_TYPE_EVENT_DATA_MAIN_SESSION_VISIBILITY_CHANGED_EXTX,
      "XR_TYPE_EVENT_DATA_MAIN_SESSION_VISIBILITY_CHANGED_EXTX" },
    { XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB, "XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB" },
    { XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPATIAL_ANCHOR_CREATE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_PASSTHROUGH_STATE_CHANGED_FB, "XR_TYPE_EVENT_DATA_PASSTHROUGH_STATE_CHANGED_FB" },
    { XR_TYPE_EVENT_DATA_MARKER_TRACKING_UPDATE_VARJO, "XR_TYPE_EVENT_DATA_MARKER_TRACKING_UPDATE_VARJO" },
    { XR_TYPE_EVENT_DATA_LOCALIZATION_CHANGED_ML, "XR_TYPE_EVENT_DATA_LOCALIZATION_CHANGED_ML" },
    { XR_TYPE_EVENT_DATA_HEADSET_FIT_CHANGED_ML, "XR_TYPE_EVENT_DATA_HEADSET_FIT_CHANGED_ML" },
    { XR_TYPE_EVENT_DATA_EYE_CALIBRATION_CHANGED_ML, "XR_TYPE_EVENT_DATA_EYE_CALIBRATION_CHANGED_ML" },
    { XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB, "XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB" },
    { XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_SPACE_SHARE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_SHARE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_COMMIT_TEXT_META, "XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_COMMIT_TEXT_META" },
    { XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_BACKSPACE_META, "XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_BACKSPACE_META" },
    { XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_ENTER_META, "XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_ENTER_META" },
    { XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_SHOWN_META, "XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_SHOWN_META" },
    { XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_HIDDEN_META, "XR_TYPE_EVENT_DATA_VIRTUAL_KEYBOARD_HIDDEN_META" },
    { XR_TYPE_EVENT_DATA_SPACE_LIST_SAVE_COMPLETE_FB, "XR_TYPE_EVENT_DATA_SPACE_LIST_SAVE_COMPLETE_FB" },
    { XR_TYPE_EVENT_DATA_USER_PRESENCE_CHANGED_EXT, "XR_TYPE_EVENT_DATA_USER_PRESENCE_CHANGED_EXT" },
};

bool IsHandledEventType(XrStructureType type)
{
    size_t len = sizeof(events_to_string) / sizeof(EventStrings);
    for (size_t ii = 0; ii < len; ++ii)
    {
        if (type == events_to_string[ii].type)
        {
            return true;
        }
    }
    return false;
}

const char* GetEventTypeString(XrStructureType type)
{
    size_t len = sizeof(events_to_string) / sizeof(EventStrings);
    for (size_t ii = 0; ii < len; ++ii)
    {
        if (type == events_to_string[ii].type)
        {
            return events_to_string[ii].name;
        }
    }
    return events_to_string[0].name;
}

void OpenXrReplayConsumerBase::Process_xrPollEvent(const ApiCallInfo&                               call_info,
                                                   XrResult                                         returnValue,
                                                   format::HandleId                                 instance,
                                                   StructPointerDecoder<Decoded_XrEventDataBuffer>* eventData)
{
    if (returnValue != XR_SUCCESS)
    {
        // Capture did not return an event, skip
        return;
    }

    XrInstance         in_instance = MapHandle<OpenXrInstanceInfo>(instance, &CommonObjectInfoTable::GetXrInstanceInfo);
    XrEventDataBuffer* capture_event = eventData->GetPointer();

    // We received events that haven't been handled yet already, so see if this one is in the list already
    for (size_t ii = 0; ii < received_events_.size(); ++ii)
    {
        if (received_events_[ii].type == capture_event->type)
        {
            GFXRECON_LOG_WARNING("Previously received event %s (0x%x, %u)",
                                 GetEventTypeString(capture_event->type),
                                 capture_event->type,
                                 capture_event->type);
            received_events_.erase(received_events_.begin() + ii);
            return;
        }
    }

    // We ignored previous events that were not indicated in the capture yet, so see if this one is in the list already
    for (size_t ii = 0; ii < received_events_.size(); ++ii)
    {
        if (skipped_unhandled_events_[ii].type == capture_event->type)
        {
            GFXRECON_LOG_WARNING("Previously recorded but skipped event %s (0x%x, %u)",
                                 GetEventTypeString(capture_event->type),
                                 capture_event->type,
                                 capture_event->type);
            skipped_unhandled_events_.erase(skipped_unhandled_events_.begin() + ii);
            return;
        }
    }

    XrEventDataBuffer* out_eventData =
        eventData->IsNull() ? nullptr : eventData->AllocateOutputData(1, { XR_TYPE_EVENT_DATA_BUFFER, nullptr });
    InitializeOutputStructNext(eventData);

    XrResult replay_result;

    // WIP: Put this constant somewhere interesting
    const uint32_t kRetryLimit      = 16;
    const int64_t  kMaxSleepLimitNs = 500000000; // 500ms
    uint32_t       retry_count      = 0;

    if (out_eventData && capture_event)
    {
        int64_t sleep_time = 1;
        do
        {
            *out_eventData = XrEventDataBuffer{ XR_TYPE_EVENT_DATA_BUFFER };
            replay_result  = GetInstanceTable(in_instance)->PollEvent(in_instance, out_eventData);
            retry_count++;

            GFXRECON_LOG_INFO("Looking for event %s (0x%x, %u)",
                              GetEventTypeString(capture_event->type),
                              capture_event->type,
                              capture_event->type);
            if (capture_event->type != out_eventData->type)
            {
                if (replay_result == XR_SUCCESS)
                {
                    // Add it to the list of received events
                    received_events_.push_back(*out_eventData);
                    GFXRECON_LOG_WARNING("Recording event for later %s (0x%x, %u)",
                                         GetEventTypeString(out_eventData->type),
                                         out_eventData->type,
                                         out_eventData->type);

                    // If we grow too large on the event vector, it's probably because we have
                    // received a bunch of events we can not handle.  So remove the first 100
                    // events to make room for more without bloating the list size.
                    // TODO: Perhaps do this more elegantly?
                    if (received_events_.size() > 1000)
                    {
                        GFXRECON_LOG_WARNING("Received event list is now %d in size, stripping the first 100!",
                                             received_events_.size());
                        received_events_.erase(received_events_.begin(), received_events_.begin() + 100);
                    }
                }
                else if (replay_result == XR_EVENT_UNAVAILABLE)
                {
                    // Yield and retry
                    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_time));

                    if (sleep_time < kMaxSleepLimitNs)
                    {
                        // Next time, sleep for double what we initially slept for.  This way if we're just
                        // spinning, we spin less and less each time.
                        sleep_time *= 2;
                    }
                    // Clamp it to the max (should stay here from this point forward)
                    if (sleep_time > kMaxSleepLimitNs)
                    {
                        sleep_time = kMaxSleepLimitNs;
                    }
                }
                else
                {
                    GFXRECON_LOG_ERROR("xrPollEvent encountered an error of type 0x%x", replay_result);
                    break;
                }
            }
        } while ((retry_count < kRetryLimit) && capture_event->type != out_eventData->type);
        if (capture_event->type != out_eventData->type)
        {
            if (IsHandledEventType(capture_event->type))
            {
                GFXRECON_LOG_ERROR("Event %s (0x%x %d) never occurred!",
                                   GetEventTypeString(capture_event->type),
                                   capture_event->type,
                                   capture_event->type);

                // Runtime never gave us the event we were looking for
                replay_result = XR_ERROR_RUNTIME_FAILURE;
            }
            else
            {
                GFXRECON_LOG_WARNING("Unhandled Event %s (0x%x %d) never occurred, pretending everything is still fine",
                                     GetEventTypeString(capture_event->type),
                                     capture_event->type,
                                     capture_event->type);

                // Save the event just in case we receive it in the future
                skipped_unhandled_events_.push_back(*capture_event);

                // If we grow too large on the event vector, it's probably because we have
                // received a bunch of events we can not handle.  So remove the first 100
                // events to make room for more without bloating the list size.
                // TODO: Perhaps do this more elegantly?
                if (skipped_unhandled_events_.size() > 1000)
                {
                    GFXRECON_LOG_WARNING("Unhandled Event list is now %d in size, stripping the first 100!",
                                         skipped_unhandled_events_.size());
                    skipped_unhandled_events_.erase(skipped_unhandled_events_.begin(),
                                                    skipped_unhandled_events_.begin() + 100);
                }

                // Return the expected replay code just because we don't know how to handle this and
                // we never received it (just so we don't assert out).
                replay_result = returnValue;
            }
        }
    }
    else
    {
        // Event data can't be null
        replay_result = XR_ERROR_VALIDATION_FAILURE;
    }

    CheckResult("xrPollEvent", returnValue, replay_result, call_info);
}

void OpenXrReplayConsumerBase::UpdateState_xrWaitFrame(const ApiCallInfo&                             call_info,
                                                       XrResult                                       returnValue,
                                                       format::HandleId                               session,
                                                       StructPointerDecoder<Decoded_XrFrameWaitInfo>* frameWaitInfo,
                                                       StructPointerDecoder<Decoded_XrFrameState>*    frameState,
                                                       XrResult                                       replay_result)
{

    // Store wait frame information for this session if needed later
    SessionData& session_data = GetSessionData(session);
    session_data.SetDisplayTime(frameState->GetOutputPointer()->predictedDisplayTime);
}

void OpenXrReplayConsumerBase::UpdateState_xrEnumerateReferenceSpaces(const ApiCallInfo&        call_info,
                                                                      XrResult                  returnValue,
                                                                      format::HandleId          session,
                                                                      uint32_t                  spaceCapacityInput,
                                                                      PointerDecoder<uint32_t>* spaceCountOutput,
                                                                      PointerDecoder<XrReferenceSpaceType>* spaces,
                                                                      XrResult replay_result)
{
    // Store wait frame information for this session if needed later
    uint32_t*             out_spaceCountOutput = spaceCountOutput->GetOutputPointer();
    XrReferenceSpaceType* out_spaces           = spaces->GetOutputPointer();

    if (out_spaceCountOutput && *out_spaceCountOutput && out_spaces)
    {
        SessionData& session_data = GetSessionData(session);
        session_data.AddReferenceSpaces(*out_spaceCountOutput, out_spaces);
    }
}

void OpenXrReplayConsumerBase::Process_xrCreateSwapchain(
    const ApiCallInfo&                                   call_info,
    XrResult                                             returnValue,
    format::HandleId                                     session,
    StructPointerDecoder<Decoded_XrSwapchainCreateInfo>* createInfo,
    HandlePointerDecoder<XrSwapchain>*                   swapchain)
{
    XrSession in_session = MapHandle<OpenXrSessionInfo>(session, &CommonObjectInfoTable::GetXrSessionInfo);
    const XrSwapchainCreateInfo* in_createInfo = createInfo->GetPointer();
    if (!swapchain->IsNull())
    {
        swapchain->SetHandleLength(1);
    }
    XrSwapchain* out_swapchain = swapchain->GetHandlePointer();

    XrSwapchainCreateInfo amended_info = *in_createInfo;
    amended_info.usageFlags |= XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;

    XrResult replay_result = GetInstanceTable(in_session)->CreateSwapchain(in_session, &amended_info, out_swapchain);
    CheckResult("xrCreateSwapchain", returnValue, replay_result, call_info);

    AddHandle<OpenXrSwapchainInfo>(
        session, swapchain->GetPointer(), out_swapchain, &CommonObjectInfoTable::AddXrSwapchainInfo);

    AssociateParent(*out_swapchain, in_session);

    SessionData&   session_data = GetSessionData(session);
    SwapchainData& swap_data    = AddSwapchainData(*swapchain->GetPointer());

    swap_data.InitSwapchainData(session_data.GetGraphicsBinding(), amended_info, *out_swapchain);
}

void OpenXrReplayConsumerBase::UpdateState_xrEnumerateSwapchainImages(
    const ApiCallInfo&                                        call_info,
    XrResult                                                  returnValue,
    format::HandleId                                          swapchain,
    uint32_t                                                  imageCapacityInput,
    PointerDecoder<uint32_t>*                                 imageCountOutput,
    StructPointerDecoder<Decoded_XrSwapchainImageBaseHeader>* images,
    XrResult                                                  replay_result)
{
    // When there's nothing to do, do nothing
    if (!XR_SUCCEEDED(returnValue) || (imageCapacityInput == 0) || (images->GetOutputLength() == 0))
    {
        return;
    }

    SwapchainData& swapchain_data = GetSwapchainData(swapchain);

    XrResult result = swapchain_data.ImportReplaySwapchain(images);
    if (XR_SUCCEEDED(result))
    {
        result = swapchain_data.InitVirtualSwapchain(imageCountOutput, images);
    }

    if (!XR_SUCCEEDED(result))
    {
        GFXRECON_LOG_FATAL("API call at index: %d thread: %d virtual swapchain initialzation returned error value %s.  "
                           "Replay cannot continue.",
                           call_info.index,
                           call_info.thread_id,
                           util::ToString<XrResult>(result).c_str());
        RaiseFatalError(enumutil::GetResultDescription(result));
    }
}

void OpenXrReplayConsumerBase::UpdateState_xrAcquireSwapchainImage(
    const ApiCallInfo&                                         call_info,
    XrResult                                                   returnValue,
    format::HandleId                                           swapchain,
    StructPointerDecoder<Decoded_XrSwapchainImageAcquireInfo>* acquireInfo,
    PointerDecoder<uint32_t>*                                  index,
    XrResult                                                   replay_result)
{
    uint32_t    capture_index = *index->GetPointer();
    uint32_t    out_index     = *index->GetOutputPointer();

    SwapchainData& swapchain_data = GetSwapchainData(swapchain);
    replay_result        = swapchain_data.AcquireSwapchainImage(capture_index, out_index);
}

void OpenXrReplayConsumerBase::Process_xrReleaseSwapchainImage(
    const ApiCallInfo&                                         call_info,
    XrResult                                                   returnValue,
    format::HandleId                                           swapchain,
    StructPointerDecoder<Decoded_XrSwapchainImageReleaseInfo>* releaseInfo)
{
    XrSwapchain in_swapchain = MapHandle<OpenXrSwapchainInfo>(swapchain, &CommonObjectInfoTable::GetXrSwapchainInfo);
    const XrSwapchainImageReleaseInfo* in_releaseInfo = releaseInfo->GetPointer();

    SwapchainData& swapchain_data = GetSwapchainData(swapchain);
    swapchain_data.ReleaseSwapchainImage(releaseInfo);

    XrResult replay_result = GetInstanceTable(in_swapchain)->ReleaseSwapchainImage(in_swapchain, in_releaseInfo);
    CheckResult("xrReleaseSwapchainImage", returnValue, replay_result, call_info);
}

void OpenXrReplayConsumerBase::ProcessViewRelativeLocation(format::ThreadId              thread_id,
                                                           format::ViewRelativeLocation& location)
{
    // Create a proxy space for a given space_id at a view relative location
    XrSession replay_session =
        MapHandle<OpenXrSessionInfo>(location.session_id, &CommonObjectInfoTable::GetXrSessionInfo);
    XrSpace replay_space = MapHandle<OpenXrSpaceInfo>(location.space_id, &CommonObjectInfoTable::GetXrSpaceInfo);
    assert(replay_session != XR_NULL_HANDLE);
    assert(replay_space != XR_NULL_HANDLE);
    SessionData& session_data = GetSessionData(location.session_id);
    session_data.AddViewRelativeProxySpace(GetInstanceTable(session_data.GetHandle()), location, replay_space);
}

void OpenXrReplayConsumerBase::Process_xrEndFrame(const ApiCallInfo&                            call_info,
                                                  XrResult                                      returnValue,
                                                  format::HandleId                              session,
                                                  StructPointerDecoder<Decoded_XrFrameEndInfo>* frameEndInfo)
{
    XrSession             in_session = MapHandle<OpenXrSessionInfo>(session, &CommonObjectInfoTable::GetXrSessionInfo);
    const XrFrameEndInfo* in_frameEndInfo = frameEndInfo->GetPointer();
    MapStructHandles(frameEndInfo->GetMetaStructPointer(), GetObjectInfoTable());

    XrFrameEndInfo replay_frame_end_info = *in_frameEndInfo;
    SessionData&   session_data          = GetSessionData(session);

    // The display time must be based on the time given by the runtime at replay time, as the recorded diplayTime
    // may not be a valid time at replay.
    //
    // A first approximation of this is simply using the XrFrameState::predictedDisplayTime
    //
    // NOTE: A closer approximation of the capture would be having the displayTime be at the same offset of
    // the replay predictedDisplayTime, as the recorded displayTime has from the recorded predictedDisplayTime.
    replay_frame_end_info.displayTime = session_data.GetDisplayTime();

    // TODO: Control this with a command line parameter
    session_data.RemapFrameEndSpaces(replay_frame_end_info);

    XrResult replay_result = GetInstanceTable(in_session)->EndFrame(in_session, &replay_frame_end_info);
    CheckResult("xrEndFrame", returnValue, replay_result, call_info);
    CustomProcess<format::ApiCallId::ApiCall_xrEndFrame>::UpdateState(
        this, call_info, returnValue, session, frameEndInfo, replay_result);
}

void* OpenXrReplayConsumerBase::PreProcessExternalObject(uint64_t          object_id,
                                                         format::ApiCallId call_id,
                                                         const char*       call_name)
{
    void* object = nullptr;

    if (call_id == format::ApiCallId::ApiCall_xrCreateSwapchainAndroidSurfaceKHR)
    {
        // TODO: Return jobject
    }
    else if (format::ApiCallId::ApiCall_xrCreateSpatialAnchorFromPerceptionAnchorMSFT)
    {
        // TODO: Return perceptionAnchor
    }
    else
    {
        GFXRECON_LOG_WARNING("Skipping object handle mapping for unsupported external object type processed by %s",
                             call_name);
    }

    return object;
}

void OpenXrReplayConsumerBase::PostProcessExternalObject(
    XrResult replay_result, uint64_t object_id, void* object, format::ApiCallId call_id, const char* call_name)
{
    GFXRECON_UNREFERENCED_PARAMETER(replay_result);
    GFXRECON_UNREFERENCED_PARAMETER(object_id);
    GFXRECON_UNREFERENCED_PARAMETER(object);
    GFXRECON_UNREFERENCED_PARAMETER(call_id);
    GFXRECON_UNREFERENCED_PARAMETER(call_name);
}

void OpenXrReplayConsumerBase::Process_xrLocateSpaces(const ApiCallInfo&                                call_info,
                                                      XrResult                                          returnValue,
                                                      format::HandleId                                  session,
                                                      StructPointerDecoder<Decoded_XrSpacesLocateInfo>* locateInfo,
                                                      StructPointerDecoder<Decoded_XrSpaceLocations>*   spaceLocations)
{
    XrSession in_session = MapHandle<OpenXrSessionInfo>(session, &CommonObjectInfoTable::GetXrSessionInfo);
    const XrSpacesLocateInfo* in_locateInfo = locateInfo->GetPointer();
    MapStructHandles(locateInfo->GetMetaStructPointer(), GetObjectInfoTable());
    XrSpaceLocations* out_spaceLocations =
        spaceLocations->IsNull() ? nullptr
                                 : spaceLocations->AllocateOutputData(1, { XR_TYPE_SPACE_LOCATIONS, nullptr });
    InitializeOutputStructNext(spaceLocations);

    // We have to create allocated space for the space location data to be written to, otherwise,
    // it will try to write  to a non-existent output location.
    if (out_spaceLocations != nullptr)
    {
        XrSpaceLocations*         in_spaceLocations   = spaceLocations->GetPointer();
        Decoded_XrSpaceLocations* meta_spaceLocations = spaceLocations->GetMetaStructPointer();

        out_spaceLocations->locationCount = in_spaceLocations->locationCount;
        out_spaceLocations->locations     = nullptr;
        if (in_spaceLocations->locationCount > 0 && in_spaceLocations->locations != nullptr)
        {
            out_spaceLocations->locations =
                meta_spaceLocations->locations->AllocateOutputData(out_spaceLocations->locationCount);
        }
    }

    XrResult replay_result = GetInstanceTable(in_session)->LocateSpaces(in_session, in_locateInfo, out_spaceLocations);
    CheckResult("xrLocateSpaces", returnValue, replay_result, call_info);
    CustomProcess<format::ApiCallId::ApiCall_xrLocateSpaces>::UpdateState(
        this, call_info, returnValue, session, locateInfo, spaceLocations, replay_result);
}

void OpenXrReplayConsumerBase::Process_xrLocateHandJointsEXT(
    const ApiCallInfo&                                       call_info,
    XrResult                                                 returnValue,
    format::HandleId                                         handTracker,
    StructPointerDecoder<Decoded_XrHandJointsLocateInfoEXT>* locateInfo,
    StructPointerDecoder<Decoded_XrHandJointLocationsEXT>*   locations)
{
    XrHandTrackerEXT in_handTracker =
        MapHandle<OpenXrHandTrackerEXTInfo>(handTracker, &CommonObjectInfoTable::GetXrHandTrackerEXTInfo);
    const XrHandJointsLocateInfoEXT* in_locateInfo = locateInfo->GetPointer();
    MapStructHandles(locateInfo->GetMetaStructPointer(), GetObjectInfoTable());
    XrHandJointLocationsEXT* out_locations =
        locations->IsNull() ? nullptr : locations->AllocateOutputData(1, { XR_TYPE_HAND_JOINT_LOCATIONS_EXT, nullptr });
    InitializeOutputStructNext(locations);

    // We have to create allocated space for the joint data to be written to, otherwise,
    // it will try to write  to a non-existent output location.
    if (out_locations != nullptr)
    {
        XrHandJointLocationsEXT*         in_locations   = locations->GetPointer();
        Decoded_XrHandJointLocationsEXT* meta_locations = locations->GetMetaStructPointer();

        out_locations->jointCount     = in_locations->jointCount;
        out_locations->jointLocations = nullptr;
        if (in_locations->jointCount > 0 && in_locations->jointLocations != nullptr)
        {
            out_locations->jointLocations =
                meta_locations->jointLocations->AllocateOutputData(out_locations->jointCount);
        }
    }

    XrResult replay_result =
        GetInstanceTable(in_handTracker)->LocateHandJointsEXT(in_handTracker, in_locateInfo, out_locations);
    CheckResult("xrLocateHandJointsEXT", returnValue, replay_result, call_info);
    CustomProcess<format::ApiCallId::ApiCall_xrLocateHandJointsEXT>::UpdateState(
        this, call_info, returnValue, handTracker, locateInfo, locations, replay_result);
}

void OpenXrReplayConsumerBase::Process_xrGetHandMeshFB(const ApiCallInfo&                                  call_info,
                                                       XrResult                                            returnValue,
                                                       format::HandleId                                    handTracker,
                                                       StructPointerDecoder<Decoded_XrHandTrackingMeshFB>* mesh)
{
    XrHandTrackerEXT in_handTracker =
        MapHandle<OpenXrHandTrackerEXTInfo>(handTracker, &CommonObjectInfoTable::GetXrHandTrackerEXTInfo);
    XrHandTrackingMeshFB* out_mesh =
        mesh->IsNull() ? nullptr : mesh->AllocateOutputData(1, { XR_TYPE_HAND_TRACKING_MESH_FB, nullptr });
    InitializeOutputStructNext(mesh);

    // We have to create allocated space for the mesh data to be written to, otherwise,
    // it will try to write  to a non-existent output location.
    if (out_mesh != nullptr)
    {
        XrHandTrackingMeshFB*         in_mesh   = mesh->GetPointer();
        Decoded_XrHandTrackingMeshFB* meta_mesh = mesh->GetMetaStructPointer();

        out_mesh->jointCapacityInput = in_mesh->jointCapacityInput;
        out_mesh->jointBindPoses     = nullptr;
        out_mesh->jointRadii         = nullptr;
        out_mesh->jointParents       = nullptr;
        if (in_mesh->jointCapacityInput > 0)
        {
            if (in_mesh->jointBindPoses != nullptr)
            {
                out_mesh->jointBindPoses = meta_mesh->jointBindPoses->AllocateOutputData(out_mesh->jointCapacityInput);
            }
            if (in_mesh->jointRadii != nullptr)
            {
                out_mesh->jointRadii = meta_mesh->jointRadii.AllocateOutputData(out_mesh->jointCapacityInput);
            }
            if (in_mesh->jointParents != nullptr)
            {
                out_mesh->jointParents = meta_mesh->jointParents.AllocateOutputData(out_mesh->jointCapacityInput);
            }
        }

        out_mesh->vertexCapacityInput = in_mesh->vertexCapacityInput;
        out_mesh->vertexPositions     = nullptr;
        out_mesh->vertexNormals       = nullptr;
        out_mesh->vertexUVs           = nullptr;
        out_mesh->vertexBlendIndices  = nullptr;
        out_mesh->vertexBlendWeights  = nullptr;
        if (in_mesh->vertexCapacityInput > 0)
        {
            if (in_mesh->vertexPositions != nullptr)
            {
                out_mesh->vertexPositions =
                    meta_mesh->vertexPositions->AllocateOutputData(out_mesh->vertexCapacityInput);
            }
            if (in_mesh->vertexNormals != nullptr)
            {
                out_mesh->vertexNormals = meta_mesh->vertexNormals->AllocateOutputData(out_mesh->vertexCapacityInput);
            }
            if (in_mesh->vertexUVs != nullptr)
            {
                out_mesh->vertexUVs = meta_mesh->vertexUVs->AllocateOutputData(out_mesh->vertexCapacityInput);
            }
            if (in_mesh->vertexBlendIndices != nullptr)
            {
                out_mesh->vertexBlendIndices =
                    meta_mesh->vertexBlendIndices->AllocateOutputData(out_mesh->vertexCapacityInput);
            }
            if (in_mesh->vertexBlendWeights != nullptr)
            {
                out_mesh->vertexBlendWeights =
                    meta_mesh->vertexBlendWeights->AllocateOutputData(out_mesh->vertexCapacityInput);
            }
        }

        out_mesh->indexCapacityInput = in_mesh->indexCapacityInput;
        out_mesh->indices            = nullptr;
        if (in_mesh->indexCapacityInput > 0 && in_mesh->indices != nullptr)
        {
            out_mesh->indices = meta_mesh->indices.AllocateOutputData(out_mesh->indexCapacityInput, 0);
        }
    }

    XrResult replay_result = GetInstanceTable(in_handTracker)->GetHandMeshFB(in_handTracker, out_mesh);
    CheckResult("xrGetHandMeshFB", returnValue, replay_result, call_info);
    CustomProcess<format::ApiCallId::ApiCall_xrGetHandMeshFB>::UpdateState(
        this, call_info, returnValue, handTracker, mesh, replay_result);
}

void OpenXrReplayConsumerBase::Process_xrLocateBodyJointsFB(
    const ApiCallInfo&                                      call_info,
    XrResult                                                returnValue,
    format::HandleId                                        bodyTracker,
    StructPointerDecoder<Decoded_XrBodyJointsLocateInfoFB>* locateInfo,
    StructPointerDecoder<Decoded_XrBodyJointLocationsFB>*   locations)
{
    XrBodyTrackerFB in_bodyTracker =
        MapHandle<OpenXrBodyTrackerFBInfo>(bodyTracker, &CommonObjectInfoTable::GetXrBodyTrackerFBInfo);
    const XrBodyJointsLocateInfoFB* in_locateInfo = locateInfo->GetPointer();
    MapStructHandles(locateInfo->GetMetaStructPointer(), GetObjectInfoTable());
    XrBodyJointLocationsFB* out_locations =
        locations->IsNull() ? nullptr : locations->AllocateOutputData(1, { XR_TYPE_BODY_JOINT_LOCATIONS_FB, nullptr });
    InitializeOutputStructNext(locations);

    // We have to create allocated space for the joint data to be written to, otherwise,
    // it will try to write  to a non-existent output location.
    if (out_locations != nullptr)
    {
        XrBodyJointLocationsFB*         in_locations   = locations->GetPointer();
        Decoded_XrBodyJointLocationsFB* meta_locations = locations->GetMetaStructPointer();

        out_locations->jointCount     = in_locations->jointCount;
        out_locations->jointLocations = nullptr;
        if (in_locations->jointCount > 0 && in_locations->jointLocations != nullptr)
        {
            out_locations->jointLocations =
                meta_locations->jointLocations->AllocateOutputData(out_locations->jointCount);
        }
    }

    XrResult replay_result =
        GetInstanceTable(in_bodyTracker)->LocateBodyJointsFB(in_bodyTracker, in_locateInfo, out_locations);
    CheckResult("xrLocateBodyJointsFB", returnValue, replay_result, call_info);
    CustomProcess<format::ApiCallId::ApiCall_xrLocateBodyJointsFB>::UpdateState(
        this, call_info, returnValue, bodyTracker, locateInfo, locations, replay_result);
}

void OpenXrReplayConsumerBase::UpdateState_xrGetVulkanGraphicsDeviceKHR(
    const ApiCallInfo&                      call_info,
    XrResult                                returnValue,
    format::HandleId                        instance,
    format::HandleId                        systemId,
    format::HandleId                        vulkan_instance,
    HandlePointerDecoder<VkPhysicalDevice>* vkPhysicalDevice,
    XrResult                                replay_result)
{
    if (XR_SUCCEEDED(replay_result))
    {
        VulkanPhysicalDeviceInfo* vulkan_physical_device_info =
            GetObjectInfoTable().GetVkPhysicalDeviceInfo(*vkPhysicalDevice->GetPointer());
        assert(vulkan_physical_device_info); // We call this just after we insert it
        vulkan_replay_consumer_->SetPhysicalDeviceAlias(vulkan_instance, *vulkan_physical_device_info);
    }
}

void OpenXrReplayConsumerBase::UpdateState_xrGetVulkanGraphicsDevice2KHR(
    const ApiCallInfo&                                              call_info,
    XrResult                                                        returnValue,
    format::HandleId                                                instance,
    StructPointerDecoder<Decoded_XrVulkanGraphicsDeviceGetInfoKHR>* getInfo,
    HandlePointerDecoder<VkPhysicalDevice>*                         vulkanPhysicalDevice,
    XrResult                                                        replay_result)
{
    if (XR_SUCCEEDED(replay_result))
    {
        const Decoded_XrVulkanGraphicsDeviceGetInfoKHR* decoded_info = getInfo->GetMetaStructPointer();
        assert(decoded_info);
        const format::HandleId vulkan_instance = decoded_info->vulkanInstance;

        VulkanPhysicalDeviceInfo* vulkan_physical_device_info =
            GetObjectInfoTable().GetVkPhysicalDeviceInfo(*vulkanPhysicalDevice->GetPointer());
        assert(vulkan_physical_device_info); // We call this just after we insert it

        vulkan_replay_consumer_->SetPhysicalDeviceAlias(vulkan_instance, *vulkan_physical_device_info);
    }
}

void OpenXrReplayConsumerBase::CheckResult(const char*                func_name,
                                           XrResult                   original,
                                           XrResult                   replay,
                                           const decode::ApiCallInfo& call_info,
                                           bool                       assert_on_error)
{
    if (original != replay)
    {
        if (replay < 0)
        {
            if (assert_on_error)
            {
                // Raise a fatal error if replay produced an error that did not occur during capture.  Format not
                // supported errors are not treated as fatal, but will be reported as warnings below, allowing the
                // replay to attempt to continue for the case where an application may have queried for formats that it
                // did not use.
                GFXRECON_LOG_FATAL("API call at index: %d thread: %d %s returned error value %s that does not match "
                                   "the result from the "
                                   "capture file: %s. Replay cannot continue.",
                                   call_info.index,
                                   call_info.thread_id,
                                   func_name,
                                   util::ToString<XrResult>(replay).c_str(),
                                   util::ToString<XrResult>(original).c_str());

                RaiseFatalError(enumutil::GetResultDescription(replay));
            }
            else
            {
                GFXRECON_LOG_ERROR("API call at index: %d thread: %d %s returned error value %s that does not match "
                                   "the result from the capture file: %s.",
                                   call_info.index,
                                   call_info.thread_id,
                                   func_name,
                                   util::ToString<XrResult>(replay).c_str(),
                                   util::ToString<XrResult>(original).c_str());
            }
        }
        else if (!((replay == XR_SUCCESS) &&
                   ((original == XR_TIMEOUT_EXPIRED) || (original == XR_SESSION_LOSS_PENDING) ||
                    (original == XR_EVENT_UNAVAILABLE) || (original == XR_SPACE_BOUNDS_UNAVAILABLE) ||
                    (original == XR_SESSION_NOT_FOCUSED) || (original == XR_FRAME_DISCARDED))))
        {
            // Report differences between replay result and capture result, unless the replay results indicates
            // that a wait operation completed before the original or a WSI function succeeded when the original failed.
            GFXRECON_LOG_WARNING(
                "API call %s returned value %s that does not match return value from capture file: %s.",
                func_name,
                util::ToString<XrResult>(replay).c_str(),
                util::ToString<XrResult>(original).c_str());
        }
    }
}

void OpenXrReplayConsumerBase::RaiseFatalError(const char* message) const
{
    // TODO: Should there be a default action if no error handler has been provided?
    if (fatal_error_handler_ != nullptr)
    {
        fatal_error_handler_(message);
    }
}

openxr::GraphicsBinding OpenXrReplayConsumerBase::MakeGraphicsBinding(Decoded_XrSessionCreateInfo* create_info)
{
    auto* vk_binding = gfxrecon::decode::GetNextMetaStruct<Decoded_XrGraphicsBindingVulkanKHR>(create_info->next);
    if (vk_binding)
    {
        assert(vulkan_replay_consumer_);
        assert(vk_binding->decoded_value);

        return openxr::GraphicsBinding(openxr::VulkanGraphicsBinding(*vulkan_replay_consumer_, *vk_binding));
    }

    // Add additional bindings below this

    // Default constructed object !IsValid()
    return openxr::GraphicsBinding();
}

// Override the handling of the XrSpaceVelocities structure when found in a 'next' chain.
// The problem is that it is an output structure, but it needs initialization done for it to be
// properly filled in by the API.  This includes, setting proper array sizes, and creating
// storage space for those arrays.
XrBaseOutStructure* OverrideOutputStructNext_XrSpaceVelocities(const XrBaseInStructure* in_next,
                                                               XrBaseOutStructure*      output_struct)
{
    XrSpaceVelocities* out_space_velocities = DecodeAllocator::Allocate<XrSpaceVelocities>();
    if (out_space_velocities != nullptr)
    {
        const XrSpaceVelocities* in_space_velocities = reinterpret_cast<const XrSpaceVelocities*>(in_next);
        out_space_velocities->velocityCount          = in_space_velocities->velocityCount;
        if (out_space_velocities->velocityCount > 0)
        {
            out_space_velocities->velocities =
                DecodeAllocator::Allocate<XrSpaceVelocityData>(in_space_velocities->velocityCount);
            memcpy(out_space_velocities->velocities,
                   in_space_velocities->velocities,
                   sizeof(XrSpaceVelocityData) * in_space_velocities->velocityCount);
        }
    }
    return reinterpret_cast<XrBaseOutStructure*>(out_space_velocities);
}

// Override the handling of the XrBindingModificationsKHR structure when found in a 'next' chain.
// The problem is that it is an output structure, but it needs initialization done for it to be
// properly filled in by the API.  This includes, setting proper array sizes, and creating
// storage space for those arrays.  Unfortunately, this is complicated by the fact that some of the
// arrays of structures are for "BaseHeader" type place-holders which need to be deciphered, and then
// those also have arrays of content as well.
XrBaseOutStructure* OverrideOutputStructNext_XrBindingModificationsKHR(const XrBaseInStructure* in_next,
                                                                       XrBaseOutStructure*      output_struct)
{
    XrBindingModificationsKHR* out_binding_mod_parent = DecodeAllocator::Allocate<XrBindingModificationsKHR>();
    if (out_binding_mod_parent != nullptr)
    {
        const XrBindingModificationsKHR* in_binding_mod_parent =
            reinterpret_cast<const XrBindingModificationsKHR*>(in_next);
        out_binding_mod_parent->type                     = in_binding_mod_parent->type;
        out_binding_mod_parent->next                     = nullptr;
        out_binding_mod_parent->bindingModificationCount = in_binding_mod_parent->bindingModificationCount;
        if (in_binding_mod_parent->bindingModificationCount > 0)
        {
            XrBindingModificationBaseHeaderKHR** out_binding_mods =
                DecodeAllocator::Allocate<XrBindingModificationBaseHeaderKHR*>(
                    in_binding_mod_parent->bindingModificationCount);
            const XrBindingModificationBaseHeaderKHR* const* in_binding_mods =
                in_binding_mod_parent->bindingModifications;
            for (uint32_t iii = 0; iii < in_binding_mod_parent->bindingModificationCount; ++iii)
            {
                switch (in_binding_mods[iii]->type)
                {
                    default:
                        out_binding_mods[iii]       = DecodeAllocator::Allocate<XrBindingModificationBaseHeaderKHR>();
                        out_binding_mods[iii]->type = in_binding_mods[iii]->type;
                        GFXRECON_LOG_ERROR(
                            "Unknown bindingModification structure type %u for XrBindingModificationsKHR index %u",
                            in_binding_mods[iii]->type,
                            iii);
                        break;
                    case XR_TYPE_INTERACTION_PROFILE_DPAD_BINDING_EXT:
                    {
                        XrInteractionProfileDpadBindingEXT* out_dpad_binding =
                            DecodeAllocator::Allocate<XrInteractionProfileDpadBindingEXT>();
                        const XrInteractionProfileDpadBindingEXT* in_dpad_binding =
                            reinterpret_cast<const XrInteractionProfileDpadBindingEXT*>(in_binding_mods[iii]);
                        memcpy(out_dpad_binding, in_dpad_binding, sizeof(XrInteractionProfileDpadBindingEXT));
                        if (out_dpad_binding->onHaptic != nullptr)
                        {
                            switch (in_dpad_binding->onHaptic->type)
                            {
                                default:
                                {
                                    XrHapticBaseHeader* out_haptic_base =
                                        DecodeAllocator::Allocate<XrHapticBaseHeader>();
                                    out_haptic_base->type = in_dpad_binding->onHaptic->type;
                                    GFXRECON_LOG_ERROR(
                                        "Unknown onHaptic structure type %u for XrBindingModificationsKHR index %u",
                                        in_dpad_binding->onHaptic->type,
                                        iii);
                                    out_dpad_binding->onHaptic = out_haptic_base;
                                    break;
                                }
                                case XR_TYPE_HAPTIC_VIBRATION:
                                {
                                    XrHapticVibration* out_haptic_vib = DecodeAllocator::Allocate<XrHapticVibration>();
                                    memcpy(out_haptic_vib, in_dpad_binding->onHaptic, sizeof(XrHapticVibration));
                                    out_dpad_binding->onHaptic = reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                                case XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB:
                                {
                                    XrHapticAmplitudeEnvelopeVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticAmplitudeEnvelopeVibrationFB>();
                                    const XrHapticAmplitudeEnvelopeVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticAmplitudeEnvelopeVibrationFB*>(
                                            in_dpad_binding->onHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticAmplitudeEnvelopeVibrationFB));

                                    if (out_haptic_vib->amplitudeCount > 0)
                                    {
                                        float* out_amplitudes =
                                            DecodeAllocator::Allocate<float>(in_haptic_vib->amplitudeCount);
                                        memcpy(out_amplitudes,
                                               in_haptic_vib->amplitudes,
                                               sizeof(float) * in_haptic_vib->amplitudeCount);
                                        out_haptic_vib->amplitudes = out_amplitudes;
                                    }

                                    out_dpad_binding->onHaptic = reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                                case XR_TYPE_HAPTIC_PCM_VIBRATION_FB:
                                {
                                    XrHapticPcmVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticPcmVibrationFB>();
                                    const XrHapticPcmVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticPcmVibrationFB*>(in_dpad_binding->onHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticPcmVibrationFB));

                                    if (in_haptic_vib->samplesConsumed != nullptr)
                                    {
                                        out_haptic_vib->samplesConsumed  = DecodeAllocator::Allocate<uint32_t>();
                                        *out_haptic_vib->samplesConsumed = *in_haptic_vib->samplesConsumed;
                                    }

                                    if (in_haptic_vib->bufferSize > 0)
                                    {
                                        float* out_buffer = DecodeAllocator::Allocate<float>(in_haptic_vib->bufferSize);
                                        memcpy(out_buffer,
                                               in_haptic_vib->buffer,
                                               sizeof(float) * in_haptic_vib->bufferSize);
                                        out_haptic_vib->buffer = out_buffer;
                                    }

                                    out_dpad_binding->onHaptic = reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                            }
                        }
                        if (out_dpad_binding->offHaptic != nullptr)
                        {
                            switch (out_dpad_binding->offHaptic->type)
                            {
                                default:
                                {
                                    XrHapticBaseHeader* out_haptic_base =
                                        DecodeAllocator::Allocate<XrHapticBaseHeader>();
                                    out_haptic_base->type = in_dpad_binding->offHaptic->type;
                                    GFXRECON_LOG_ERROR(
                                        "Unknown offHaptic structure type %u for XrBindingModificationsKHR index %u",
                                        in_dpad_binding->offHaptic->type,
                                        iii);
                                    out_dpad_binding->offHaptic = out_haptic_base;
                                    break;
                                }
                                case XR_TYPE_HAPTIC_VIBRATION:
                                {
                                    XrHapticVibration* out_haptic_vib = DecodeAllocator::Allocate<XrHapticVibration>();
                                    memcpy(out_haptic_vib, in_dpad_binding->offHaptic, sizeof(XrHapticVibration));
                                    break;
                                }
                                case XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB:
                                {
                                    XrHapticAmplitudeEnvelopeVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticAmplitudeEnvelopeVibrationFB>();
                                    const XrHapticAmplitudeEnvelopeVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticAmplitudeEnvelopeVibrationFB*>(
                                            in_dpad_binding->offHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticAmplitudeEnvelopeVibrationFB));

                                    if (in_haptic_vib->amplitudeCount > 0)
                                    {
                                        float* out_amplitudes =
                                            DecodeAllocator::Allocate<float>(in_haptic_vib->amplitudeCount);
                                        memcpy(out_amplitudes,
                                               in_haptic_vib->amplitudes,
                                               sizeof(float) * in_haptic_vib->amplitudeCount);
                                        out_haptic_vib->amplitudes = out_amplitudes;
                                    }

                                    out_dpad_binding->offHaptic = reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                                case XR_TYPE_HAPTIC_PCM_VIBRATION_FB:
                                {
                                    XrHapticPcmVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticPcmVibrationFB>();
                                    const XrHapticPcmVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticPcmVibrationFB*>(in_dpad_binding->offHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticPcmVibrationFB));

                                    if (in_haptic_vib->samplesConsumed != nullptr)
                                    {
                                        out_haptic_vib->samplesConsumed  = DecodeAllocator::Allocate<uint32_t>();
                                        *out_haptic_vib->samplesConsumed = *in_haptic_vib->samplesConsumed;
                                    }

                                    if (in_haptic_vib->bufferSize > 0)
                                    {
                                        float* out_buffer = DecodeAllocator::Allocate<float>(in_haptic_vib->bufferSize);
                                        ;
                                        memcpy(out_buffer,
                                               in_haptic_vib->buffer,
                                               sizeof(float) * in_haptic_vib->bufferSize);
                                        out_haptic_vib->buffer = out_buffer;
                                    }

                                    out_dpad_binding->offHaptic = reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                            }
                        }
                        out_binding_mods[iii] = reinterpret_cast<XrBindingModificationBaseHeaderKHR*>(out_dpad_binding);
                        break;
                    }

                    case XR_TYPE_INTERACTION_PROFILE_ANALOG_THRESHOLD_VALVE:
                    {
                        XrInteractionProfileAnalogThresholdVALVE* out_thresh_binding =
                            DecodeAllocator::Allocate<XrInteractionProfileAnalogThresholdVALVE>();
                        const XrInteractionProfileAnalogThresholdVALVE* in_thresh_binding =
                            reinterpret_cast<const XrInteractionProfileAnalogThresholdVALVE*>(in_binding_mods[iii]);
                        memcpy(out_thresh_binding, in_thresh_binding, sizeof(XrInteractionProfileAnalogThresholdVALVE));
                        if (out_thresh_binding->onHaptic != nullptr)
                        {
                            switch (out_thresh_binding->onHaptic->type)
                            {
                                default:
                                {
                                    XrHapticBaseHeader* out_haptic_base =
                                        DecodeAllocator::Allocate<XrHapticBaseHeader>();
                                    out_haptic_base->type = in_thresh_binding->onHaptic->type;
                                    GFXRECON_LOG_ERROR(
                                        "Unknown onHaptic structure type %u for XrBindingModificationsKHR index %u",
                                        in_thresh_binding->onHaptic->type,
                                        iii);
                                    out_thresh_binding->onHaptic = out_haptic_base;
                                    break;
                                }
                                case XR_TYPE_HAPTIC_VIBRATION:
                                {
                                    XrHapticVibration* out_haptic_vib = DecodeAllocator::Allocate<XrHapticVibration>();
                                    memcpy(out_haptic_vib, in_thresh_binding->onHaptic, sizeof(XrHapticVibration));
                                    break;
                                }
                                case XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB:
                                {
                                    XrHapticAmplitudeEnvelopeVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticAmplitudeEnvelopeVibrationFB>();
                                    const XrHapticAmplitudeEnvelopeVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticAmplitudeEnvelopeVibrationFB*>(
                                            in_thresh_binding->onHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticAmplitudeEnvelopeVibrationFB));

                                    if (in_haptic_vib->amplitudeCount > 0)
                                    {
                                        float* out_amplitudes =
                                            DecodeAllocator::Allocate<float>(in_haptic_vib->amplitudeCount);
                                        memcpy(out_amplitudes,
                                               in_haptic_vib->amplitudes,
                                               sizeof(float) * in_haptic_vib->amplitudeCount);
                                        out_haptic_vib->amplitudes = out_amplitudes;
                                    }

                                    out_thresh_binding->onHaptic =
                                        reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                                case XR_TYPE_HAPTIC_PCM_VIBRATION_FB:
                                {
                                    XrHapticPcmVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticPcmVibrationFB>();
                                    const XrHapticPcmVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticPcmVibrationFB*>(in_thresh_binding->onHaptic);
                                    memcpy(out_haptic_vib, in_thresh_binding->onHaptic, sizeof(XrHapticPcmVibrationFB));

                                    if (in_haptic_vib->samplesConsumed != nullptr)
                                    {
                                        out_haptic_vib->samplesConsumed  = DecodeAllocator::Allocate<uint32_t>();
                                        *out_haptic_vib->samplesConsumed = *in_haptic_vib->samplesConsumed;
                                    }

                                    if (in_haptic_vib->bufferSize > 0)
                                    {
                                        float* out_buffer = DecodeAllocator::Allocate<float>(in_haptic_vib->bufferSize);
                                        memcpy(out_buffer,
                                               in_haptic_vib->buffer,
                                               sizeof(float) * in_haptic_vib->bufferSize);
                                        out_haptic_vib->buffer = out_buffer;
                                    }

                                    out_thresh_binding->onHaptic =
                                        reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                            }
                        }
                        if (out_thresh_binding->offHaptic != nullptr)
                        {
                            switch (out_thresh_binding->offHaptic->type)
                            {
                                default:
                                {
                                    XrHapticBaseHeader* out_haptic_base =
                                        DecodeAllocator::Allocate<XrHapticBaseHeader>();
                                    out_haptic_base->type = in_thresh_binding->offHaptic->type;
                                    GFXRECON_LOG_ERROR(
                                        "Unknown offHaptic structure type %u for XrBindingModificationsKHR index %u",
                                        in_thresh_binding->offHaptic->type,
                                        iii);
                                    out_thresh_binding->offHaptic = out_haptic_base;
                                    break;
                                }
                                case XR_TYPE_HAPTIC_VIBRATION:
                                {
                                    XrHapticVibration* out_haptic_vib = DecodeAllocator::Allocate<XrHapticVibration>();
                                    memcpy(out_haptic_vib, in_thresh_binding->offHaptic, sizeof(XrHapticVibration));
                                    break;
                                }
                                case XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB:
                                {
                                    XrHapticAmplitudeEnvelopeVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticAmplitudeEnvelopeVibrationFB>();
                                    const XrHapticAmplitudeEnvelopeVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticAmplitudeEnvelopeVibrationFB*>(
                                            in_thresh_binding->offHaptic);
                                    memcpy(out_haptic_vib, in_haptic_vib, sizeof(XrHapticAmplitudeEnvelopeVibrationFB));

                                    if (in_haptic_vib->amplitudeCount > 0)
                                    {
                                        float* out_amplitudes =
                                            DecodeAllocator::Allocate<float>(in_haptic_vib->amplitudeCount);
                                        memcpy(out_amplitudes,
                                               in_haptic_vib->amplitudes,
                                               sizeof(float) * in_haptic_vib->amplitudeCount);
                                        out_haptic_vib->amplitudes = out_amplitudes;
                                    }

                                    out_thresh_binding->offHaptic =
                                        reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                                case XR_TYPE_HAPTIC_PCM_VIBRATION_FB:
                                {
                                    XrHapticPcmVibrationFB* out_haptic_vib =
                                        DecodeAllocator::Allocate<XrHapticPcmVibrationFB>();
                                    const XrHapticPcmVibrationFB* in_haptic_vib =
                                        reinterpret_cast<const XrHapticPcmVibrationFB*>(in_thresh_binding->offHaptic);
                                    memcpy(
                                        out_haptic_vib, in_thresh_binding->offHaptic, sizeof(XrHapticPcmVibrationFB));

                                    if (in_haptic_vib->samplesConsumed != nullptr)
                                    {
                                        out_haptic_vib->samplesConsumed  = DecodeAllocator::Allocate<uint32_t>();
                                        *out_haptic_vib->samplesConsumed = *in_haptic_vib->samplesConsumed;
                                    }

                                    if (in_haptic_vib->bufferSize > 0)
                                    {
                                        float* out_buffer = DecodeAllocator::Allocate<float>(in_haptic_vib->bufferSize);
                                        memcpy(out_buffer,
                                               in_haptic_vib->buffer,
                                               sizeof(float) * in_haptic_vib->bufferSize);
                                        out_haptic_vib->buffer = out_buffer;
                                    }

                                    out_thresh_binding->offHaptic =
                                        reinterpret_cast<XrHapticBaseHeader*>(out_haptic_vib);
                                    break;
                                }
                            }
                        }
                        out_binding_mods[iii] =
                            reinterpret_cast<XrBindingModificationBaseHeaderKHR*>(out_thresh_binding);
                        break;
                    }
                }
            }
            out_binding_mod_parent->bindingModifications =
                reinterpret_cast<const XrBindingModificationBaseHeaderKHR* const*>(out_binding_mods);
        }
    }
    return reinterpret_cast<XrBaseOutStructure*>(out_binding_mod_parent);
}

// Override the handling of the XrHandJointVelocitiesEXT structure when found in a 'next' chain.
// The problem is that it is an output structure, but it needs initialization done for it to be
// properly filled in by the API.  This includes, setting proper array sizes, and creating
// storage space for those arrays.
XrBaseOutStructure* OverrideOutputStructNext_XrHandJointVelocitiesEXT(const XrBaseInStructure* in_next,
                                                                      XrBaseOutStructure*      output_struct)
{
    XrHandJointVelocitiesEXT* out_hand_joint_velocities = DecodeAllocator::Allocate<XrHandJointVelocitiesEXT>();
    if (out_hand_joint_velocities != nullptr)
    {
        const XrHandJointVelocitiesEXT* in_hand_joint_velocities =
            reinterpret_cast<const XrHandJointVelocitiesEXT*>(in_next);
        out_hand_joint_velocities->jointCount = in_hand_joint_velocities->jointCount;
        if (out_hand_joint_velocities->jointCount > 0)
        {
            out_hand_joint_velocities->jointVelocities =
                DecodeAllocator::Allocate<XrHandJointVelocityEXT>(in_hand_joint_velocities->jointCount);
            memcpy(out_hand_joint_velocities->jointVelocities,
                   in_hand_joint_velocities->jointVelocities,
                   sizeof(XrHandJointVelocityEXT) * in_hand_joint_velocities->jointCount);
        }
    }
    return reinterpret_cast<XrBaseOutStructure*>(out_hand_joint_velocities);
}

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // ENABLE_OPENXR_SUPPORT
