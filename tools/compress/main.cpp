/*
** Copyright (c) 2018-2020 Valve Corporation
** Copyright (c) 2018-2020 LunarG, Inc.
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

#include PROJECT_VERSION_HEADER_FILE
#include "compression_converter.h"

#include "decode/file_processor.h"
#include "format/format.h"
#include "util/compressor.h"
#include "util/logging.h"

#include "vulkan/vulkan_core.h"

#include "generated_compress_settings.h"

#include <cassert>
#include <cstdlib>

const char kArgNone[]    = "NONE";
const char kArgLz4[]     = "LZ4";
const char kArgZlib[]    = "ZLIB";
const char kArgZstd[]    = "ZSTD";
const char kArgUnknown[] = "<Unknown>";

static std::string GetCompressionTypeName(uint32_t type)
{
    switch (type)
    {
        case gfxrecon::format::CompressionType::kNone:
            return kArgNone;
        case gfxrecon::format::CompressionType::kLz4:
            return kArgLz4;
        case gfxrecon::format::CompressionType::kZlib:
            return kArgZlib;
        case gfxrecon::format::CompressionType::kZstd:
            return kArgZstd;
        default:
            break;
    }

    return kArgUnknown;
}

int main(int argc, const char** argv)
{
    // Create the tool settings using a smart pointer so it is automatically cleaned up on exit
    std::unique_ptr<gfxrecon::tools::ToolSettings> tool_settings =
        std::make_unique<gfxrecon::tools::ToolSettings>(gfxrecon::util::settings::kGfxrToolType_Compress_Tool);

    std::vector<std::string>        extra_args;
    gfxrecon::tools::CmdLineRetType ret_type = tool_settings->ProcessCommandLine(argc, argv, 3, extra_args);

    if (ret_type == gfxrecon::tools::CmdLineRetType_PrintUsage)
    {
        PrintUsage(argv[0]);
        exit(0);
    }
    else if (ret_type == gfxrecon::tools::CmdLineRetType_PrintVersion)
    {
        gfxrecon::tools::PrintVersion(argv[0]);
        exit(0);
    }
    else if (ret_type == gfxrecon::tools::CmdLineRetType_Error)
    {
        PrintUsage(argv[0]);
        exit(-1);
    }
    tool_settings->ProcessDisableDebugPopup();

    std::string                       input_filename         = extra_args[0];
    std::string                       output_filename        = extra_args[1];
    std::string                       dst_compression_string = extra_args[2];
    gfxrecon::format::CompressionType compression_type        = gfxrecon::format::kNone;

    if (gfxrecon::util::platform::StringCompareNoCase(kArgNone, dst_compression_string.c_str()) != 0)
    {
        bool compression_set = false;

#if defined(GFXRECON_ENABLE_LZ4_COMPRESSION)
        if (gfxrecon::util::platform::StringCompareNoCase(kArgLz4, dst_compression_string.c_str()) == 0)
        {
            compression_type = gfxrecon::format::CompressionType::kLz4;
            compression_set = true;
        }
#endif
#if defined(GFXRECON_ENABLE_ZLIB_COMPRESSION)
        if (gfxrecon::util::platform::StringCompareNoCase(kArgZlib, dst_compression_string.c_str()) == 0)
        {
            compression_type = gfxrecon::format::CompressionType::kZlib;
            compression_set = true;
        }
#endif
#if defined(GFXRECON_ENABLE_ZSTD_COMPRESSION)
        if (gfxrecon::util::platform::StringCompareNoCase(kArgZstd, dst_compression_string.c_str()) == 0)
        {
            compression_type = gfxrecon::format::CompressionType::kZstd;
            compression_set = true;
        }
#endif
        if (!compression_set)
        {
            GFXRECON_LOG_ERROR("Unsupported compression format \'%s\'", dst_compression_string.c_str());
            PrintUsage(argv[0]);
            gfxrecon::util::Log::Release();
            exit(-1);
        }
    }

    gfxrecon::CompressionConverter file_converter;

    if (file_converter.Initialize(input_filename, output_filename, compression_type))
    {
        if (file_converter.Process())
        {
            std::string src_compression = kArgNone;

            for (const auto& option : file_converter.GetFileOptions())
            {
                if (option.key == gfxrecon::format::kCompressionType)
                {
                    src_compression = GetCompressionTypeName(option.value);

                    if (src_compression == kArgUnknown)
                    {
                        GFXRECON_LOG_ERROR("Unrecognized source compression type %u", option.value);
                    }
                }
            }

            if (gfxrecon::format::CompressionType::kNone != compression_type)
            {
                std::string dst_compression = GetCompressionTypeName(compression_type);
                uint64_t    bytes_read      = file_converter.GetNumBytesRead();
                uint64_t    bytes_written   = file_converter.GetNumBytesWritten();
                float       percent_reduction =
                    100.f * (1.f - (static_cast<float>(bytes_written) / static_cast<float>(bytes_read)));
                GFXRECON_WRITE_CONSOLE("Compression Results:");
                GFXRECON_WRITE_CONSOLE(
                    "  Original Size   [Compression: %5s] = %" PRIu64 " bytes", src_compression.c_str(), bytes_read);
                GFXRECON_WRITE_CONSOLE(
                    "  Compressed Size [Compression: %5s] = %" PRIu64 " bytes", dst_compression.c_str(), bytes_written);
                GFXRECON_WRITE_CONSOLE("  Percent Reduction                    = %.2f%%", percent_reduction);
            }
            else
            {
                uint64_t bytes_read    = file_converter.GetNumBytesRead();
                uint64_t bytes_written = file_converter.GetNumBytesWritten();
                float    percent_increase =
                    100.f * ((static_cast<float>(bytes_written) / static_cast<float>(bytes_read)) - 1.f);
                GFXRECON_WRITE_CONSOLE("Decompression Results:");
                GFXRECON_WRITE_CONSOLE(
                    "  Original Size   [Compression: %5s] = %" PRIu64 " bytes", src_compression.c_str(), bytes_read);
                GFXRECON_WRITE_CONSOLE("  Uncompressed Size                    = %" PRIu64 " bytes", bytes_written);
                GFXRECON_WRITE_CONSOLE("  Percent Increase                     = %.2f%%", percent_increase);
            }
        }
        else
        {
            GFXRECON_WRITE_CONSOLE("Capture file %s could not be converted.", input_filename.c_str());
            gfxrecon::util::Log::Release();
            exit(-1);
        }
    }
    else
    {
        GFXRECON_WRITE_CONSOLE("CompressionConverter could not be initialized.");
        gfxrecon::util::Log::Release();
        exit(-1);
    }

    gfxrecon::util::Log::Release();

    return 0;
}
