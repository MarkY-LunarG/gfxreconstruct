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

#ifndef GFXRECON_REPLAY_D3D12_FEATURE_H
#define GFXRECON_REPLAY_D3D12_FEATURE_H

#if defined(D3D12_SUPPORT)

#include "generated/generated_dx12_decoder.h"
#include "generated/generated_dx12_replay_consumer.h"
#include "decode/dx12_pre_process_consumer.h"
#ifdef GFXRECON_AGS_SUPPORT
#include "decode/custom_ags_consumer_base.h"
#include "decode/custom_ags_decoder.h"
#include "decode/custom_ags_replay_consumer.h"
#endif // GFXRECON_AGS_SUPPORT
#include "decode/dx12_tracking_consumer.h"
#include "graphics/dx12_util.h"

#include "replay_feature.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(replay)

class ReplayD3d12Feature : public ReplayGraphicsFeature
{
  public:
    virtual ~ReplayD3d12Feature() = default;

    // Simple "getter" style methods
    std::string Label() override { return "D3D12"; }

    void QueryOptions(util::ArgumentParser& arg_parser, const std::string& capture_filename) override;
    void RegisterDecodeComponents(decode::FileProcessor*                    file_processor,
                                  std::shared_ptr<application::Application> application,
                                  graphics::FpsInfo*                        fps_info) override;

    virtual void SetupPreProcessingPass(decode::FileProcessor* file_processor) override;
    virtual void CompletePreProcessingPass(std::string& dr_block_indices) override;

  protected:
    void InternalCleanup() override;

  private:
    decode::DxReplayOptions                     replay_options_;
    std::unique_ptr<decode::Dx12ReplayConsumer> replay_consumer_;
    decode::Dx12Decoder                         decoder_;
#ifdef GFXRECON_AGS_SUPPORT
    decode::AgsReplayConsumer ags_replay_consumer_;
    decode::AgsDecoder        ags_decoder_;
#endif // GFXRECON_AGS_SUPPORT

    std::unique_ptr<decode::FileProcessor>          pre_processor_file_processor_;
    std::unique_ptr<decode::Dx12PreProcessConsumer> pre_processor_consumer_;
    std::unique_ptr<decode::Dx12Decoder>            pre_processor_decoder_;
};

GFXRECON_END_NAMESPACE(replay)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // D3D12_SUPPORT

#endif // GFXRECON_REPLAY_D3D12_FEATURE_H
