/*
** Copyright (c) 2020-2026 LunarG, Inc.
** Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include "replay_openxr_feature.h"

#include "generated/generated_vulkan_replay_consumer.h"
#include "util/feature_module_registry.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(replay)

// Register this class as a feature in a module registry
GFXR_UTIL_REGISTER_FEATURE_CREATOR(ReplayCompositionFeature, ReplayOpenXrFeature)

void ReplayOpenXrFeature::QueryOptions(util::ArgumentParser& arg_parser, const std::string& capture_filename)
{
    capture_filename_ = capture_filename;
    is_enabled_       = true;
}

void ReplayOpenXrFeature::CreateConsumer(decode::FileProcessor*                    file_processor,
                                         std::shared_ptr<application::Application> application,
                                         gfxrecon::graphics::FrameLoopInfo*        frame_loop_info)
{
    GFXRECON_UNREFERENCED_PARAMETER(frame_loop_info);

    if (is_enabled_)
    {
        file_processor_ = file_processor;
        application_    = application;

        replay_consumer_ = std::make_unique<decode::OpenXrReplayConsumer>(application_, replay_options_);
        replay_consumer_->SetFatalErrorHandler([](const char* message) { throw std::runtime_error(message); });
    }
}

void ReplayOpenXrFeature::RegisterDecodeComponents(graphics::FpsInfo* fps_info)
{
    if (is_enabled_)
    {
        replay_consumer_->SetFpsInfo(fps_info);

        decoder_.AddConsumer(replay_consumer_.get());
        file_processor_->AddDecoder(&decoder_);
    }
}

void ReplayOpenXrFeature::SetGraphicsFeatures(
    const std::vector<std::unique_ptr<ReplayGraphicsFeature>>& graphics_features)
{
    for (auto& feature : graphics_features)
    {
        if (feature->Label() == "Vulkan")
        {
            decode::VulkanReplayConsumer* vulkan_consumer =
                reinterpret_cast<decode::VulkanReplayConsumer*>(feature->GetConsumer());
            if (vulkan_consumer != nullptr)
            {
                replay_consumer_->SetVulkanReplayConsumer(vulkan_consumer);
            }
        }
    }
}

void ReplayOpenXrFeature::PostReplay()
{
    replay_consumer_.reset();
}

GFXRECON_END_NAMESPACE(replay)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // ENABLE_OPENXR_SUPPORT
