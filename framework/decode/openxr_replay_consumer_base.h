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

#ifndef GFXRECON_DECODE_OPENXR_JSON_CONSUMER_BASE_H
#define GFXRECON_DECODE_OPENXR_JSON_CONSUMER_BASE_H

#if ENABLE_OPENXR_SUPPORT

#include <unordered_map>
#include "application/application.h"
#include "decode/openxr_handle_mapping_util.h"
#include "decode/openxr_object_info.h"
#include "decode/openxr_replay_options.h"
#include "decode/openxr_resource_tracking_consumer.h"
#include "decode/vulkan_resource_tracking_consumer.h"
#include "generated/generated_openxr_consumer.h"
#include "generated/generated_openxr_dispatch_table.h"

#include <functional>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)
class VulkanReplayConsumerBase;

class OpenXrReplayConsumerBase : public OpenXrConsumer
{
  public:
    OpenXrReplayConsumerBase(std::shared_ptr<application::Application> application, const OpenXrReplayOptions& options);

    ~OpenXrReplayConsumerBase() override;

    void SetVulkanReplayConsumer(VulkanReplayConsumerBase* vulkan_replay_consumer);

    virtual void
    Process_xrInitializeLoaderKHR(const ApiCallInfo&                                           call_info,
                                  XrResult                                                     returnValue,
                                  StructPointerDecoder<Decoded_XrLoaderInitInfoBaseHeaderKHR>* loaderInitInfo) override;

    virtual void Process_xrCreateApiLayerInstance(const ApiCallInfo&                                  call_info,
                                                  XrResult                                            returnValue,
                                                  StructPointerDecoder<Decoded_XrInstanceCreateInfo>* info,
                                                  StructPointerDecoder<Decoded_XrApiLayerCreateInfo>* apiLayerInfo,
                                                  HandlePointerDecoder<XrInstance>*) override;
    virtual void Process_xrGetSystem(const ApiCallInfo&                             call_info,
                                     XrResult                                       returnValue,
                                     format::HandleId                               instance,
                                     StructPointerDecoder<Decoded_XrSystemGetInfo>* getInfo,
                                     PointerDecoder<XrSystemId>*                    systemId);
    virtual void
                 Process_xrEnumerateViewConfigurationViews(const ApiCallInfo&        call_info,
                                                           XrResult                  returnValue,
                                                           format::HandleId          instance,
                                                           XrSystemId                systemId,
                                                           XrViewConfigurationType   viewConfigurationType,
                                                           uint32_t                  viewCapacityInput,
                                                           PointerDecoder<uint32_t>* viewCountOutput,
                                                           StructPointerDecoder<Decoded_XrViewConfigurationView>* views) override;
    virtual void Process_xrGetVulkanGraphicsRequirementsKHR(
        const ApiCallInfo&                                             call_info,
        XrResult                                                       returnValue,
        format::HandleId                                               instance,
        XrSystemId                                                     systemId,
        StructPointerDecoder<Decoded_XrGraphicsRequirementsVulkanKHR>* graphicsRequirements) override;
    virtual void
    Process_xrGetVulkanGraphicsDeviceKHR(const ApiCallInfo&                      call_info,
                                         XrResult                                returnValue,
                                         format::HandleId                        instance,
                                         XrSystemId                              systemId,
                                         format::HandleId                        vkInstance,
                                         HandlePointerDecoder<VkPhysicalDevice>* vkPhysicalDevice) override;

    void SetFatalErrorHandler(std::function<void(const char*)> handler) { fatal_error_handler_ = handler; }

    const OpenXrReplayOptions options_;

  protected:
    const CommonObjectInfoTable& GetObjectInfoTable() const { return object_info_table_; }

    CommonObjectInfoTable& GetObjectInfoTable() { return object_info_table_; }

    void AddInstanceTable(XrInstance instance);

    const encode::OpenXrInstanceTable* GetInstanceTable(XrInstance handle) const;
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    // NOTE: This won't work on 32-bit builds because OpenXR defines all 32-bit
    //       handles as uint64_t breaking the type conversion on each of these
    //       override functions.
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSession handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSpace handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSwapchain handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrActionSet handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrAction handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrDebugUtilsMessengerEXT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSpatialAnchorMSFT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrHandTrackerEXT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrBodyTrackerFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSceneObserverMSFT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSceneMSFT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrFacialTrackerHTC handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrFoveationProfileFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrPassthroughFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrPassthroughLayerFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrGeometryInstanceFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrPlaneDetectorEXT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrPassthroughHTC handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrMarkerDetectorML handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrExportedLocalizationMapML handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSpatialAnchorStoreConnectionMSFT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrFaceTrackerFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrEyeTrackerFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrVirtualKeyboardMETA handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSpaceUserFB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrPassthroughColorLutMETA handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrFaceTracker2FB handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrSpatialGraphNodeBindingMSFT handle) const;
    const encode::OpenXrInstanceTable* GetInstanceTable(XrTriangleMeshFB handle) const;
#endif // defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)

    void AssociateParent(XrSession session, XrInstance instance);
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    void AssociateParent(XrSpace space, XrSession session);
    void AssociateParent(XrSwapchain swapchain, XrSession session);
    void AssociateParent(XrActionSet action_set, XrInstance instance);
    void AssociateParent(XrAction action, XrActionSet action_set);
    void AssociateParent(XrDebugUtilsMessengerEXT debug_messenger, XrInstance instance);
    void AssociateParent(XrSpatialAnchorMSFT spatial_anchor, XrSession session);
    void AssociateParent(XrHandTrackerEXT hand_tracker, XrSession session);
    void AssociateParent(XrSpace space, XrHandTrackerEXT hand_tracker);
    void AssociateParent(XrBodyTrackerFB body_tracker, XrSession session);
    void AssociateParent(XrSceneObserverMSFT scene_observer, XrSession session);
    void AssociateParent(XrSceneMSFT scene, XrSceneObserverMSFT);
    void AssociateParent(XrFacialTrackerHTC facial_tracker, XrSession session);
    void AssociateParent(XrFoveationProfileFB foveation_profile, XrSession session);
    void AssociateParent(XrPassthroughFB passthrough, XrSession session);
    void AssociateParent(XrPassthroughLayerFB passthrough_layer, XrSession session);
    void AssociateParent(XrGeometryInstanceFB geometry_instance, XrSession session);
    void AssociateParent(XrMarkerDetectorML marker_detector, XrSession session);
    void AssociateParent(XrExportedLocalizationMapML exported_localization, XrSession session);
    void AssociateParent(XrSpatialAnchorStoreConnectionMSFT spatial_anchor_con, XrSession session);
    void AssociateParent(XrFaceTrackerFB face_tracker, XrSession session);
    void AssociateParent(XrEyeTrackerFB eye_tracker, XrSession session);
    void AssociateParent(XrVirtualKeyboardMETA virtual_keyboard, XrSession session);
    void AssociateParent(XrSpace space, XrVirtualKeyboardMETA virtual_keyboard);
    void AssociateParent(XrSpaceUserFB space_user, XrSession session);
    void AssociateParent(XrPassthroughColorLutMETA passthrough_color_lut, XrPassthroughFB);
    void AssociateParent(XrFaceTracker2FB face_tracker, XrSession session);
    void AssociateParent(XrPassthroughHTC passthrough, XrSession session);
    void AssociateParent(XrPlaneDetectorEXT plane_detector, XrSession session);
    void AssociateParent(XrSpatialGraphNodeBindingMSFT graph_node, XrSession session);
    void AssociateParent(XrTriangleMeshFB triangle_mesh, XrSession session);
#endif // defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)

    void* PreProcessExternalObject(uint64_t object_id, format::ApiCallId call_id, const char* call_name);

    void PostProcessExternalObject(
        XrResult replay_reslt, uint64_t object_id, void* object, format::ApiCallId call_id, const char* call_name);

    template <typename T>
    typename T::HandleType MapHandle(format::HandleId id,
                                     const T* (CommonObjectInfoTable::*MapFunc)(format::HandleId) const) const
    {
        return handle_mapping::MapHandle(id, object_info_table_, MapFunc);
    }

    uint64_t MapHandle(uint64_t object, XrObjectType object_type)
    {
        return handle_mapping::MapHandle(object, object_type, object_info_table_);
    }

    void RemoveHandle(format::HandleId id, void (CommonObjectInfoTable::*RemoveFunc)(format::HandleId))
    {
        handle_mapping::RemoveHandle(id, &object_info_table_, RemoveFunc);
    }

    template <typename T>
    typename T::HandleType* MapHandles(HandlePointerDecoder<typename T::HandleType>* handles_pointer,
                                       size_t                                        handles_len,
                                       const T* (CommonObjectInfoTable::*MapFunc)(format::HandleId) const) const
    {
        // This parameter is only referenced by debug builds.
        GFXRECON_UNREFERENCED_PARAMETER(handles_len);

        typename T::HandleType* handles = nullptr;

        if (handles_pointer != nullptr)
        {
            // The handle and ID array sizes are expected to be the same for mapping operations.
            assert(handles_len == handles_pointer->GetLength());

            handles = handle_mapping::MapHandleArray(handles_pointer, object_info_table_, MapFunc);
        }

        return handles;
    }

    template <typename T>
    void AddHandle(format::HandleId              parent_id,
                   const format::HandleId*       id,
                   const typename T::HandleType* handle,
                   T&&                           initial_info,
                   void (CommonObjectInfoTable::*AddFunc)(T&&))
    {
        if ((id != nullptr) && (handle != nullptr))
        {
            handle_mapping::AddHandle(
                parent_id, *id, *handle, std::forward<T>(initial_info), &object_info_table_, AddFunc);
        }
    }

    template <typename T>
    void AddHandle(format::HandleId              parent_id,
                   const format::HandleId*       id,
                   const typename T::HandleType* handle,
                   void (CommonObjectInfoTable::*AddFunc)(T&&))
    {
        if ((id != nullptr) && (handle != nullptr))
        {
            handle_mapping::AddHandle(parent_id, *id, *handle, &object_info_table_, AddFunc);
        }
    }

    template <typename T>
    void AddHandles(format::HandleId              parent_id,
                    const format::HandleId*       ids,
                    size_t                        ids_len,
                    const typename T::HandleType* handles,
                    size_t                        handles_len,
                    std::vector<T>&&              initial_infos,
                    void (CommonObjectInfoTable::*AddFunc)(T&&))
    {
        handle_mapping::AddHandleArray(
            parent_id, ids, ids_len, handles, handles_len, std::move(initial_infos), &object_info_table_, AddFunc);
    }

    template <typename T>
    void AddHandles(format::HandleId              parent_id,
                    const format::HandleId*       ids,
                    size_t                        ids_len,
                    const typename T::HandleType* handles,
                    size_t                        handles_len,
                    void (CommonObjectInfoTable::*AddFunc)(T&&))
    {
        handle_mapping::AddHandleArray(parent_id, ids, ids_len, handles, handles_len, &object_info_table_, AddFunc);
    }

    void CheckResult(const char* func_name, XrResult original, XrResult replay, const decode::ApiCallInfo& call_info);

  private:
    void RaiseFatalError(const char* message) const;

    template <typename T>
    struct OpenXrObjectInfo
    {
        using HandleType = T;
        // Standard info stored for all XR objects.
        HandleType       handle{ XR_NULL_HANDLE };            // Handle created for the object during replay.
        format::HandleId capture_id{ format::kNullHandleId }; // ID assigned to the object at capture.
        format::HandleId parent_id{ format::kNullHandleId };  // ID of the object's parent instance/device object.
    };

    template <typename T>
    struct OpenXrValueInfo
    {
        using ValueType = T;
        ValueType replay_value;  // Value returned for during replay.
        ValueType capture_value; // Value returned at capture.
    };

    template <typename HandleType>
    using HandleMap = std::unordered_map<format::HandleId, OpenXrObjectInfo<HandleType>>;
    template <typename ValueType>
    using ValueMap = std::unordered_map<ValueType, OpenXrValueInfo<ValueType>>;

    HandleMap<XrInstance>       instance_info_map_;
    HandleMap<XrSession>        session_info_map_;
    HandleMap<XrSwapchain>      swapchain_info_map_;
    ValueMap<XrSystemId>        system_id_info_map_;
    HandleMap<VkPhysicalDevice> vk_physical_device_info_map_;

    PFN_xrGetInstanceProcAddr                                   get_instance_proc_addr_;
    CommonObjectInfoTable                                       object_info_table_;
    std::unordered_map<XrInstance, encode::OpenXrInstanceTable> instance_tables_;
    std::shared_ptr<application::Application>                   application_;
    std::function<void(const char*)>                            fatal_error_handler_;

    std::unordered_map<XrSession, XrInstance>                          session_to_instance_;
    std::unordered_map<XrSpace, XrInstance>                            space_to_instance_;
    std::unordered_map<XrSwapchain, XrInstance>                        swapchain_to_instance_;
    std::unordered_map<XrActionSet, XrInstance>                        actionset_to_instance_;
    std::unordered_map<XrAction, XrInstance>                           action_to_instance_;
    std::unordered_map<XrDebugUtilsMessengerEXT, XrInstance>           debugutilsmessengerEXT_to_instance_;
    std::unordered_map<XrSpatialAnchorMSFT, XrInstance>                spatialanchorMSFT_to_instance_;
    std::unordered_map<XrHandTrackerEXT, XrInstance>                   handtrackerEXT_to_instance_;
    std::unordered_map<XrBodyTrackerFB, XrInstance>                    bodytrackerFB_to_instance_;
    std::unordered_map<XrSceneObserverMSFT, XrInstance>                sceneobserverMSFT_to_instance_;
    std::unordered_map<XrSceneMSFT, XrInstance>                        sceneMSFT_to_instance_;
    std::unordered_map<XrFacialTrackerHTC, XrInstance>                 facialtrackerHTC_to_instance_;
    std::unordered_map<XrFoveationProfileFB, XrInstance>               foveationprofileFB_to_instance_;
    std::unordered_map<XrPassthroughFB, XrInstance>                    passthroughFB_to_instance_;
    std::unordered_map<XrPassthroughLayerFB, XrInstance>               passthroughlayerFB_to_instance_;
    std::unordered_map<XrGeometryInstanceFB, XrInstance>               geometryinstanceFB_to_instance_;
    std::unordered_map<XrPlaneDetectorEXT, XrInstance>                 planedetectorEXT_to_instance_;
    std::unordered_map<XrPassthroughHTC, XrInstance>                   passthroughHTC_to_instance_;
    std::unordered_map<XrMarkerDetectorML, XrInstance>                 markerdetectorML_to_instance_;
    std::unordered_map<XrExportedLocalizationMapML, XrInstance>        exportedlocalicationML_to_instance_;
    std::unordered_map<XrSpatialAnchorStoreConnectionMSFT, XrInstance> spatialanchorconnectionMSFT_to_instance_;
    std::unordered_map<XrFaceTrackerFB, XrInstance>                    facetrackerFB_to_instance_;
    std::unordered_map<XrEyeTrackerFB, XrInstance>                     eyetrackerFB_to_instance_;
    std::unordered_map<XrVirtualKeyboardMETA, XrInstance>              virtualkeyboardMETA_to_instance_;
    std::unordered_map<XrSpaceUserFB, XrInstance>                      spaceuserFB_to_instance_;
    std::unordered_map<XrPassthroughColorLutMETA, XrInstance>          passthroughcolorlutMETA_to_instance_;
    std::unordered_map<XrFaceTracker2FB, XrInstance>                   facetracker2FB_to_instance_;
    std::unordered_map<XrSpatialGraphNodeBindingMSFT, XrInstance>      spatialgraphnodebindingMSFT_to_instance_;
    std::unordered_map<XrTriangleMeshFB, XrInstance>                   trianglemeshFB_to_instance_;

    // TODO: Should DRY the handle remapping code with the Vulkan side at least.
    template <typename HandleDecoder, typename Map>
    void AddHandleMapping(format::HandleId parent, HandleDecoder& handle_decoder, Map& map)
    {
        using HandleType               = typename HandleDecoder::HandleType;
        using Info                     = OpenXrObjectInfo<HandleType>;
        format::HandleId capture_id    = *handle_decoder.GetPointer();
        HandleType       replay_handle = *handle_decoder.GetHandlePointer();
        map.insert(std::make_pair(capture_id, Info{ replay_handle, capture_id, parent }));
    }

    template <typename PointerDecoder, typename ValueType, typename Map>
    void AddValueMapping(PointerDecoder& pointer_decoder, ValueType& replay_value, Map& map)
    {
        using Info                     = OpenXrValueInfo<ValueType>;
        const ValueType& capture_value = *pointer_decoder.GetPointer();
        map.insert(std::make_pair(capture_value, Info{ replay_value, capture_value }));
    }

    template <typename Map, typename InfoType = typename Map::mapped_type>
    InfoType* GetMappingInfo(typename Map::key_type key, Map& map)
    {
        InfoType* info     = nullptr;
        auto      found_it = map.find(key);
        if (found_it != map.end())
        {
            info = &found_it->second;
        }
        return info;
    }

    VulkanReplayConsumerBase* vulkan_replay_consumer_ = nullptr;
};

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // ENABLE_OPENXR_SUPPORT

#endif // GFXRECON_DECODE_OPENXR_JSON_CONSUMER_BASE_H