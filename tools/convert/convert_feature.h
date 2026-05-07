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

#ifndef GFXRECON_CONVERT_FEATURE_H
#define GFXRECON_CONVERT_FEATURE_H

#include "decode/file_processor.h"
#include "decode/json_writer.h"
#include "decode/marker_json_consumer.h"
#include "decode/metadata_json_consumer.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(convert)

// Base untyped interface. Used for the polymorphic feature vector in main.
class ConvertFeatureBase
{
  public:
    virtual ~ConvertFeatureBase() = default;

    // Detection pass logic
    virtual void SetupDetectionPass(decode::FileProcessor& file_processor) = 0;
    virtual void CleanupDetectionPass()                                    = 0;
    virtual bool WasDetected()                                             = 0;

    // Generate usage logic
    virtual void SetupConvertPass(decode::FileProcessor& file_processor, decode::JsonWriter* json_writer) = 0;
    virtual void CleanupConvertPass()                                                                     = 0;
};

// Template used by all API instantiations.
template <class ConsumerT, class DecoderT, class DetectConsumerT>
class ConvertFeature : public ConvertFeatureBase
{
  public:
    ~ConvertFeature()
    {
        // Cleanup both passes, just in case one or both didn't already
        CleanupDetectionPass();
        CleanupConvertPass();
    }

    virtual void InitDetectConsumer() { detect_consumer_ = new DetectConsumerT(); }

    void SetupDetectionPass(decode::FileProcessor& file_processor) final
    {
        detection_decoder_ = new DecoderT();
        if (detection_decoder_ != nullptr)
        {
            InitDetectConsumer();
            if (detect_consumer_ != nullptr)
            {
                detection_decoder_->AddConsumer(detect_consumer_);
                file_processor.AddDecoder(detection_decoder_);
            }
            else
            {
                GFXRECON_LOG_WARNING("Failed to setup detection pass consumer");
                delete detection_decoder_;
                detection_decoder_ = nullptr;
            }
        }
        else
        {
            GFXRECON_LOG_WARNING("Failed to setup detection pass decoder");
        }
    }

    virtual void CleanupDetectionPass() override
    {
        if (detection_decoder_)
        {
            delete detection_decoder_;
            detection_decoder_ = nullptr;
        }
        if (detect_consumer_)
        {
            delete detect_consumer_;
            detect_consumer_ = nullptr;
        }
    }

    void SetupConvertPass(decode::FileProcessor& file_processor, decode::JsonWriter* json_writer) final
    {
        json_consumer_.Initialize(json_writer);
        decoder_.AddConsumer(&json_consumer_);
        file_processor.AddDecoder(&decoder_);
    }

    void CleanupConvertPass() final { json_consumer_.Destroy(); }

  protected:
    ConsumerT        json_consumer_;
    DecoderT         decoder_;
    DecoderT*        detection_decoder_{ nullptr };
    DetectConsumerT* detect_consumer_{ nullptr };
};

GFXRECON_END_NAMESPACE(convert)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_CONVERT_FEATURE_H
