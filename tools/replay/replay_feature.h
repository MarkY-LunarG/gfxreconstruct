/*
** Copyright (c) 2026 LunarG, Inc.
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

#ifndef GFXRECON_REPLAY_FEATURE_H
#define GFXRECON_REPLAY_FEATURE_H

#include "util/strings.h"

#include "application/application.h"
#include "decode/file_processor.h"
#include "graphics/fps_info.h"
#include "graphics/frame_loop_info.h"
#include "util/argument_parser.h"

#include <memory>
#include <stdexcept>
#include <string>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(replay)

class ReplayFeature
{
  public:
    virtual ~ReplayFeature() { PostReplay(); }

    virtual std::string Label() = 0;

    // Options queries
    virtual void QueryOptions(util::ArgumentParser& arg_parser, const std::string& capture_filename) {}

    // Composition methods (for an API like OpenXR which uses other graphics APIs to
    // compose final images).
    bool         IsCompositingFeature() { return is_compositor_; }
    bool         IsGraphicsFeatureSupportingComposition() { return supports_composition_; }
    virtual void AddGraphicsFeatureForComposition(std::unique_ptr<ReplayFeature>& feature) {}

    // Recapture methods (capturing while replaying)
    bool         SupportsRecapture() { return supports_recapture_; }
    virtual void DetectAndSetupRecapture() {}

    // Frame and FPS methods
    bool         CanAdjustFpsInfo() { return can_adjust_fps_info_; }
    void         SetMeasurementStartFrame(uint32_t frame) { measurement_start_frame_ = frame; }
    virtual void QueryFpsInfoOptions(bool& quit_after_range,
                                     bool& flush_range,
                                     bool& flush_inside_range,
                                     bool& preload_range,
                                     bool& quit_after_frame)
    {}

    // Pre-processing functionality
    bool         NeedsPreProcessingPass() { return needs_pre_processor_; }
    virtual void SetupPreProcessingPass(decode::FileProcessor* file_processor) {}
    virtual void CompletePreProcessingPass() {}

    // Decode/Consumer functions
    virtual void  CreateConsumer(decode::FileProcessor*                    file_processor,
                                 std::shared_ptr<application::Application> application,
                                 gfxrecon::graphics::FrameLoopInfo*        frame_loop_info) = 0;
    virtual void* GetConsumer() { return nullptr; }
    virtual void  RegisterDecodeComponents(graphics::FpsInfo* fps_info) = 0;

    // Cleanup
    virtual void PostReplay() {}

  protected:
    std::string                               capture_filename_;
    std::shared_ptr<application::Application> application_;
    decode::FileProcessor*                    file_processor_{ nullptr };
    bool                                      needs_pre_processor_{ false };
    bool                                      is_enabled_{ false };
    bool                                      can_adjust_fps_info_{ false };
    uint32_t                                  measurement_start_frame_{ 0 };
    bool                                      supports_recapture_{ false };
    bool                                      is_compositor_{ false };
    bool                                      supports_composition_{ false };
};

// Template base for features with a typed consumer, decoder, and options object.
// Provides GetConsumer(), PostReplay(), and protected helpers used by CreateConsumer
// and RegisterDecodeComponents implementations.
template <typename ConsumerT, typename DecoderT, typename OptionsT>
class ReplayFeatureImpl : public ReplayFeature
{
  public:
    void* GetConsumer() override { return reinterpret_cast<void*>(replay_consumer_.get()); }
    void  PostReplay() override { replay_consumer_.reset(); }

  protected:
    OptionsT                   replay_options_;
    std::unique_ptr<ConsumerT> replay_consumer_;
    DecoderT                   decoder_;

    void InitConsumer(decode::FileProcessor* file_processor, std::shared_ptr<application::Application> application)
    {
        file_processor_ = file_processor;
        application_    = application;
    }

    void FinalizeConsumer()
    {
        replay_consumer_->SetFatalErrorHandler([](const char* message) { throw std::runtime_error(message); });
    }

    void RegisterConsumerAndDecoder(graphics::FpsInfo* fps_info)
    {
        replay_consumer_->SetFpsInfo(fps_info);
        decoder_.AddConsumer(replay_consumer_.get());
        file_processor_->AddDecoder(&decoder_);
    }
};

GFXRECON_END_NAMESPACE(replay)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_REPLAY_FEATURE_H
