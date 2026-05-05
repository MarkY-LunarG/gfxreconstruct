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

#include "replay_d3d12_feature.h"

#include "util/feature_module_registry.h"
#include "util/logging.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(replay)

// Register this class as a feature in a module registry
GFXR_UTIL_REGISTER_FEATURE_CREATOR(ReplayFeature, ReplayD3d12Feature)

void ReplayD3d12Feature::QueryOptions(gfxrecon::util::ArgumentParser& arg_parser, const std::string& capture_filename)
{
    capture_filename_ = capture_filename;

#if defined(D3D12_SUPPORT)
    replay_options_      = GetDxReplayOptions(arg_parser, capture_filename);
    is_enabled_          = replay_options_.enable_d3d12;
    needs_pre_processor_ = is_enabled_ && replay_options_.enable_dump_resources;
#endif // D3D12_SUPPORT
}

void ReplayD3d12Feature::CreateConsumer(decode::FileProcessor*                    file_processor,
                                        std::shared_ptr<application::Application> application,
                                        gfxrecon::graphics::FrameLoopInfo*        frame_loop_info)
{
    GFXRECON_UNREFERENCED_PARAMETER(frame_loop_info);

    if (is_enabled_)
    {
        file_processor_ = file_processor;
        application_    = application;

#if defined(D3D12_SUPPORT)
        application->InitializeDx12WsiContext();
        if (graphics::dx12::VerifyAgilitySDKRuntime() == false)
        {
            GFXRECON_LOG_ERROR("Did not find Agility SDK runtimes. Verify \\D3D12\\D3D12Core.dll exists in the same "
                               "directory as gfxrecon-replay.exe.");
        }

        replay_consumer_ = std::make_unique<decode::Dx12ReplayConsumer>(application_, replay_options_);
        replay_consumer_->SetFatalErrorHandler([](const char* message) { throw std::runtime_error(message); });
#endif
    }
}

void ReplayD3d12Feature::RegisterDecodeComponents(graphics::FpsInfo* fps_info)
{
    if (is_enabled_)
    {
#if defined(D3D12_SUPPORT)
        replay_consumer_->SetFpsInfo(fps_info);

        // Check for user option if first pass tracking is enabled
        if (replay_options_.enable_d3d12_two_pass_replay)
        {
            decode::FileProcessor              file_processor_tracking;
            decode::Dx12TrackedObjectInfoTable tracked_object_info_table;
            auto tracking_consumer = new decode::DX12TrackingConsumer(replay_options_, &tracked_object_info_table);
            if (file_processor_tracking.Initialize(capture_filename_))
            {
                decoder_.AddConsumer(tracking_consumer);
                file_processor_tracking.AddDecoder(&decoder_);
                file_processor_tracking.ProcessAllFrames();
                file_processor_tracking.RemoveDecoder(&decoder_);
                decoder_.RemoveConsumer(tracking_consumer);
            }
        }

        decoder_.AddConsumer(replay_consumer_.get());
        file_processor_->AddDecoder(decoder_);
#endif // D3D12_SUPPORT

#ifdef GFXRECON_AGS_SUPPORT
        ags_replay_consumer_.AddDx12Consumer(replay_consumer_.get());
        ags_decoder_.AddConsumer(reinterpret_cast<gfxrecon::decode::AgsConsumerBase*>(&ags_replay_consumer_));

        file_processor_->AddDecoder(&ags_decoder_);
#endif // GFXRECON_AGS_SUPPORT
    }
}

void ReplayD3d12Feature::SetupPreProcessingPass(decode::FileProcessor* file_processor)
{
#if defined(D3D12_SUPPORT)
    if (needs_pre_processor_)
    {
        pre_processor_consumer_ = std::make_unique<decode::Dx12PreProcessConsumer>();
        pre_processor_decoder_  = std::make_unique<decode::Dx12Decoder>();

        if (replay_options_.using_dump_resources_target)
        {
            pre_processor_consumer->EnableDumpResources(replay_options_.dump_resources_target);
        }

        pre_processor_decoder_->AddConsumer(pre_processor_consumer_.get());
        file_processor->AddDecoder(pre_processor_decoder_.get());
    }
#endif // D3D12_SUPPORT
}

void ReplayD3d12Feature::CompletePreProcessingPass()
{
#if defined(D3D12_SUPPORT)
    if (needs_pre_processor_)
    {
        replay_options_.enable_d3d12 = pre_processor_consumer_->WasD3D12APIDetected();
        if (replay_options_.enable_d3d12)
        {
            auto track_dump_target = pre_processor_consumer_->`GetTrackDumpTarget();
            GFXRECON_ASSERT(track_dump_target != nullptr);
            replay_consumer_->SetDumpTarget(*track_dump_target);
        }

        pre_processor_decoder_.reset();
        pre_processor_consumer_.reset();
    }
#endif // D3D12_SUPPORT
}

void ReplayD3d12Feature::PostReplay()
{
#if defined(D3D12_SUPPORT)
    if (is_enabled_ && (file_processor_->GetCurrentFrameNumber() >= measurement_start_frame_) &&
        (file_processor_->GetErrorState() == gfxrecon::decode::BlockIOError::kErrorNone))
    {
        replay_consumer_->PostReplay();
        if (!replay_options_.screenshot_ranges.empty() && !file_processor_->UsesFrameMarkers() &&
            (replay_consumer_->GetDXGITestPresentCount() > 0))
        {
            GFXRECON_LOG_WARNING_ONCE(
                "This capture contains %" PRIu32
                " calls to IDXGISwapChain::Present with flag DXGI_PRESENT_TEST and no frame end markers. "
                "Screenshot frame indexing may have changed since capture.",
                replay_consumer_->GetDXGITestPresentCount());
        }
    }
    replay_consumer_.reset();
#endif // D3D12_SUPPORT
}

GFXRECON_END_NAMESPACE(replay)
GFXRECON_END_NAMESPACE(gfxrecon)
