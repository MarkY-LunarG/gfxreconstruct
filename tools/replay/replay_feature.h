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
#include "util/argument_parser.h"

#include <string>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(replay)

class ReplayFeatureBase
{
  public:
    virtual ~ReplayFeatureBase() = default;

    virtual std::string Label() = 0;

    virtual void QueryOptions(util::ArgumentParser& arg_parser, const std::string& capture_filename) = 0;
    virtual void RegisterDecodeComponents(decode::FileProcessor*                    file_processor,
                                          std::shared_ptr<application::Application> application,
                                          graphics::FpsInfo*                        fps_info)                               = 0;
    virtual void PostReplay()                                                                        = 0;

  protected:
    std::string                               capture_filename_;
    std::shared_ptr<application::Application> application_;
    decode::FileProcessor*                    file_processor_{ nullptr };
    bool                                      is_enabled_{ false };
};

class ReplayGraphicsFeature : public ReplayFeatureBase
{
  public:
    virtual ~ReplayGraphicsFeature() = default;

    virtual void DetectAndSetupRecapture() { ; }

    bool          NeedsPreProcessingPass() { return needs_pre_processor_; }
    virtual void  SetupPreProcessingPass(decode::FileProcessor* file_processor) = 0;
    virtual void  CompletePreProcessingPass(std::string& dr_block_indices)      = 0;
    virtual void* GetReplayConsumer() { return nullptr; }

    void PostReplay() override
    {
        InternalCleanup();
        ShutdownRecapture();
    }

  protected:
    virtual void InternalCleanup() { ; }
    virtual void ShutdownRecapture() { ; }

    bool                   needs_pre_processor_{ false };
    decode::FileProcessor* pre_processor_file_processor_{ nullptr };
};

class ReplayCompositionFeature : public ReplayFeatureBase
{
  public:
    virtual ~ReplayCompositionFeature() = default;

    // Called after the graphics features are created so that the composition feature can
    // utilize the underlying enabled graphics feature.  This must be called before
    // RegisterDecodeComponents
    virtual void SetGraphicsFeatures(const std::vector<std::unique_ptr<ReplayGraphicsFeature>>& graphics_features) = 0;
};

GFXRECON_END_NAMESPACE(replay)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_REPLAY_FEATURE_H
