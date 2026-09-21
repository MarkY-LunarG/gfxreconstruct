/*
** Copyright (c) 2020-2024 LunarG, Inc.
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

#ifndef GFXRECON_DECODE_COMMON_HANDLE_MAPPING_UTIL_H
#define GFXRECON_DECODE_COMMON_HANDLE_MAPPING_UTIL_H

#include "decode/common_object_info_table.h"
#include "decode/parameter_decode_error.h"
#include "format/format.h"
#include "util/defines.h"
#include "util/logging.h"

#include <cassert>
#include <string>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)
GFXRECON_BEGIN_NAMESPACE(handle_mapping)

// The text of the message for an id that is not in the object table.
inline std::string UnmappedObjectMessage(const char* call_name, const char* type_name, format::HandleId id)
{
    std::string message = std::string(call_name) + " names a " + type_name;
    if (id == format::kNullHandleId)
    {
        message += " that is null";
    }
    else
    {
        message += " with id " + std::to_string(id) + " that no call created or that a call destroyed";
    }
    return message;
}

// The first parameter of a call names the object that selects the dispatch table and the replay
// state, so the call cannot run when that object is not in the table. A generated replay consumer
// calls this after it maps that parameter, with the mapped handle or the object info, and returns
// when the result is false. A mapped value equal to its default is not in the table. The message
// goes to ParameterDecodeError, so the dispatch visitor fails the block with its index instead of
// the call dereferencing a null object.
template <typename Mapped>
static bool DispatchObjectIsMapped(const char* call_name, const char* type_name, format::HandleId id, Mapped mapped)
{
    if (mapped != Mapped{})
    {
        return true;
    }
    ParameterDecodeError::AddPendingMessage(UnmappedObjectMessage(call_name, type_name, id));
    return false;
}

// A later parameter of a call names an object that the replay override reads. A null id is a null
// handle, which the call may allow, so it passes. An id that is not in the table is an object that
// no call created or that a call destroyed, and the call cannot be made as recorded, so the message
// goes to ParameterDecodeError and the generated consumer returns when the result is false.
template <typename Info>
static bool ObjectIsMappedOrNull(const char* call_name, const char* type_name, format::HandleId id, const Info* info)
{
    if ((id == format::kNullHandleId) || (info != nullptr))
    {
        return true;
    }
    ParameterDecodeError::AddPendingMessage(UnmappedObjectMessage(call_name, type_name, id));
    return false;
}

// The same test for a destroy or free call, whose override accepts a null object and passes a null
// handle to the driver. The call proceeds, and the log names the id, so a corrupt capture is seen.
template <typename Info>
static void WarnIfObjectIsUnmapped(const char* call_name, const char* type_name, format::HandleId id, const Info* info)
{
    if ((id != format::kNullHandleId) && (info == nullptr))
    {
        GFXRECON_LOG_WARNING("%s, so the call proceeds with a null handle",
                             UnmappedObjectMessage(call_name, type_name, id).c_str());
    }
}

template <typename T>
static typename T::HandleType MapHandle(format::HandleId             id,
                                        const CommonObjectInfoTable& object_info_table,
                                        const T* (CommonObjectInfoTable::*GetInfoFunc)(format::HandleId) const)
{
    typename T::HandleType handle = 0;

    if (id != format::kNullHandleId)
    {
        const T* info = (object_info_table.*GetInfoFunc)(id);

        if (info != nullptr)
        {
            handle = info->handle;
        }
        else
        {
            GFXRECON_LOG_WARNING("Failed to map handle for object id %" PRIu64, id);
        }
    }
    return handle;
}

template <typename T>
static typename T::HandleType* MapHandleArray(HandlePointerDecoder<typename T::HandleType>* handles_pointer,
                                              const CommonObjectInfoTable&                  object_info_table,
                                              const T* (CommonObjectInfoTable::*GetInfoFunc)(format::HandleId) const)
{
    assert(handles_pointer != nullptr);

    typename T::HandleType* handles = nullptr;

    if (!handles_pointer->IsNull())
    {
        size_t                  len = handles_pointer->GetLength();
        const format::HandleId* ids = handles_pointer->GetPointer();

        handles_pointer->SetHandleLength(len);

        handles = handles_pointer->GetHandlePointer();

        for (size_t i = 0; i < len; ++i)
        {
            if (ids[i] != format::kNullHandleId)
            {
                const T* info = (object_info_table.*GetInfoFunc)(ids[i]);
                if (info != nullptr)
                {
                    handles[i] = info->handle;
                }
                else
                {
                    handles[i] = 0;
                    GFXRECON_LOG_WARNING("Failed to map handle for object id %" PRIu64, ids[i]);
                }
            }
        }
    }

    return handles;
}

template <typename T>
static void AddHandle(format::HandleId             parent_id,
                      format::HandleId             id,
                      const typename T::HandleType handle,
                      T&&                          initial_info,
                      CommonObjectInfoTable*       object_info_table,
                      void (CommonObjectInfoTable::*AddFunc)(T&&))
{
    assert(object_info_table != nullptr);

    initial_info.handle     = handle;
    initial_info.capture_id = id;
    initial_info.parent_id  = parent_id;
    (object_info_table->*AddFunc)(std::forward<T>(initial_info));
}

template <typename T>
static void AddHandle(format::HandleId       parent_id,
                      format::HandleId       id,
                      typename T::HandleType handle,
                      CommonObjectInfoTable* object_info_table,
                      void (CommonObjectInfoTable::*AddFunc)(T&&))
{
    assert(object_info_table != nullptr);

    T info;
    info.handle     = handle;
    info.capture_id = id;
    info.parent_id  = parent_id;
    (object_info_table->*AddFunc)(std::move(info));
}

template <typename T>
static void AddHandleArray(format::HandleId              parent_id,
                           const format::HandleId*       ids,
                           size_t                        ids_len,
                           const typename T::HandleType* handles,
                           size_t                        handles_len,
                           std::vector<T>&&              initial_infos,
                           CommonObjectInfoTable*        object_info_table,
                           void (CommonObjectInfoTable::*AddFunc)(T&&))
{
    assert(object_info_table != nullptr);

    if ((ids != nullptr) && (handles != nullptr))
    {
        size_t len = std::min(ids_len, handles_len);

        assert(len <= initial_infos.size());

        for (size_t i = 0; i < len; ++i)
        {
            auto info_iter        = std::next(initial_infos.begin(), i);
            info_iter->handle     = handles[i];
            info_iter->capture_id = ids[i];
            info_iter->parent_id  = parent_id;
            (object_info_table->*AddFunc)(std::move(*info_iter));
        }
    }
}

template <typename T>
static void AddHandleArray(format::HandleId              parent_id,
                           const format::HandleId*       ids,
                           size_t                        ids_len,
                           const typename T::HandleType* handles,
                           size_t                        handles_len,
                           CommonObjectInfoTable*        object_info_table,
                           void (CommonObjectInfoTable::*AddFunc)(T&&))
{
    assert(object_info_table != nullptr);

    if ((ids != nullptr) && (handles != nullptr))
    {
        size_t len = std::min(ids_len, handles_len);
        for (size_t i = 0; i < len; ++i)
        {
            T info;
            info.handle     = handles[i];
            info.capture_id = ids[i];
            info.parent_id  = parent_id;
            (object_info_table->*AddFunc)(std::move(info));
        }
    }
}

template <typename T>
static void AddHandleArrayAsync(format::HandleId        parent_id,
                                const format::HandleId* ids,
                                size_t                  ids_len,
                                CommonObjectInfoTable*  object_info_table,
                                std::vector<T>&&        initial_infos,
                                void (CommonObjectInfoTable::*AddFunc)(T&&),
                                std::shared_future<handle_create_result_t<typename T::HandleType>> future)
{
    static_assert(has_handle_future_v<T>, "handle-type does not support asynchronous creation");
    assert(object_info_table != nullptr);

    if (ids != nullptr)
    {
        assert(ids_len <= initial_infos.size());

        // future_handle_index below is uint32_t, so the length must be bounded
        const auto ids_len_32 = GFXRECON_NARROWING_CAST(uint32_t, ids_len);
        for (uint32_t i = 0; i < ids_len_32; ++i)
        {
            auto& initial_info               = initial_infos[i];
            initial_info.handle              = VK_NULL_HANDLE; // handle does not yet exist
            initial_info.capture_id          = ids[i];
            initial_info.parent_id           = parent_id;
            initial_info.future              = future;
            initial_info.future_handle_index = i;
            (object_info_table->*AddFunc)(std::move(initial_info));
        }
    }
}

inline void RemoveHandle(format::HandleId       id,
                         CommonObjectInfoTable* object_info_table,
                         void (CommonObjectInfoTable::*RemoveFunc)(format::HandleId))
{
    assert(object_info_table != nullptr);

    if (id != format::kNullHandleId)
    {
        (object_info_table->*RemoveFunc)(id);
    }
}

GFXRECON_END_NAMESPACE(handle_mapping)
GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_DECODE_COMMON_HANDLE_MAPPING_UTIL_H
