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

#include "capture_mutations.h"

#include "format/api_call_id.h"
#include "format/format.h"
#include "format/format_util.h"
#include "util/compressor.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

using namespace gfxrecon;

namespace
{

struct MutationName
{
    CaptureMutation mutation;
    const char*     name;
};

const MutationName kNames[] = {
    { CaptureMutation::kTruncatedAtBlock, "truncated-at-block" },
    { CaptureMutation::kTruncatedInBlock, "truncated-in-block" },
    { CaptureMutation::kBadBlockSize, "bad-block-size" },
    { CaptureMutation::kBadMagic, "bad-magic" },
    { CaptureMutation::kBadVersion, "bad-version" },
    { CaptureMutation::kBadCompressedPayload, "bad-compressed-payload" },
    { CaptureMutation::kHandleNeverCreated, "handle-never-created" },
    { CaptureMutation::kSecondHandleNeverCreated, "second-handle-never-created" },
    { CaptureMutation::kParameterBufferShortByOne, "parameter-buffer-short-by-one" },
    { CaptureMutation::kParameterBufferShortByHalf, "parameter-buffer-short-by-half" },
    { CaptureMutation::kCountBomb, "count-bomb" },
    { CaptureMutation::kUnknownApiCallId, "unknown-api-call-id" },
    { CaptureMutation::kUnknownStructureType, "unknown-structure-type" },
    { CaptureMutation::kDrawBeforeBeginCommandBuffer, "draw-before-begin-command-buffer" },
    { CaptureMutation::kAnnotationLabelPastBlock, "annotation-label-past-block" },
    { CaptureMutation::kCompressionFlagOnUncompressed, "compression-flag-on-uncompressed" },
    { CaptureMutation::kMissingStateSetup, "missing-state-setup" },
};

// One block of the source file, as bytes. The file is a list of these after the file header and
// its options, so a mutation that removes, moves or rewrites a block edits the list.
struct Block
{
    std::vector<uint8_t> bytes; // Block header included.

    format::BlockHeader header() const
    {
        format::BlockHeader block_header{};
        std::memcpy(&block_header, bytes.data(), sizeof(block_header));
        return block_header;
    }
    format::BlockType base_type() const { return format::RemoveCompressedBlockBit(header().type); }
    bool              is_compressed() const { return format::IsBlockCompressed(header().type); }
    bool              is_function_call() const { return base_type() == format::BlockType::kFunctionCallBlock; }
    format::ApiCallId api_call_id() const
    {
        format::ApiCallId id{};
        std::memcpy(&id, bytes.data() + sizeof(format::BlockHeader), sizeof(id));
        return id;
    }
};

struct CaptureFile
{
    std::vector<uint8_t>              prefix; // File header and options.
    std::vector<Block>                blocks;
    std::unique_ptr<util::Compressor> compressor;
};

// The block that the structure mutations damage. Far enough in that the file header, the options
// and the first calls are intact, and early enough that a short file still holds it.
const size_t kTargetBlock = 40;

// The parameter buffer of a function call block, decompressed when the block is compressed.
struct DecodedCall
{
    format::ApiCallId    api_call_id{};
    format::ThreadId     thread_id{};
    std::vector<uint8_t> parameters;
};

bool read_file(const std::filesystem::path& source, CaptureFile& file, std::string& error)
{
    std::ifstream input{ source, std::ios::binary };
    if (!input.is_open())
    {
        error = "could not open " + source.string();
        return false;
    }
    std::vector<uint8_t> data{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };

    if (data.size() < sizeof(format::FileHeader))
    {
        error = "the file is shorter than a file header";
        return false;
    }
    format::FileHeader file_header{};
    std::memcpy(&file_header, data.data(), sizeof(file_header));
    if (file_header.fourcc != GFXRECON_FOURCC)
    {
        error = "the source is not a capture file";
        return false;
    }
    const size_t prefix_size = sizeof(format::FileHeader) + file_header.num_options * sizeof(format::FileOptionPair);
    if (data.size() < prefix_size)
    {
        error = "the file ends inside the file options";
        return false;
    }
    file.prefix.assign(data.begin(), data.begin() + prefix_size);

    format::CompressionType compression = format::CompressionType::kNone;
    for (uint32_t i = 0; i < file_header.num_options; ++i)
    {
        format::FileOptionPair option{};
        std::memcpy(&option, data.data() + sizeof(format::FileHeader) + i * sizeof(option), sizeof(option));
        if (option.key == format::FileOption::kCompressionType)
        {
            compression = static_cast<format::CompressionType>(option.value);
        }
    }
    file.compressor.reset(format::CreateCompressor(compression));

    size_t offset = prefix_size;
    while (offset + sizeof(format::BlockHeader) <= data.size())
    {
        format::BlockHeader block_header{};
        std::memcpy(&block_header, data.data() + offset, sizeof(block_header));
        const size_t total = sizeof(block_header) + block_header.size;
        if (offset + total > data.size())
        {
            error = "the source ends inside a block";
            return false;
        }
        Block block;
        block.bytes.assign(data.begin() + offset, data.begin() + offset + total);
        file.blocks.push_back(std::move(block));
        offset += total;
    }
    if (offset != data.size())
    {
        error = "the source does not end on a block boundary";
        return false;
    }
    return true;
}

bool write_file(const CaptureFile& file, const std::filesystem::path& destination, std::string& error)
{
    std::ofstream output{ destination, std::ios::binary | std::ios::trunc };
    if (!output.is_open())
    {
        error = "could not write " + destination.string();
        return false;
    }
    output.write(reinterpret_cast<const char*>(file.prefix.data()), static_cast<std::streamsize>(file.prefix.size()));
    for (const Block& block : file.blocks)
    {
        output.write(reinterpret_cast<const char*>(block.bytes.data()),
                     static_cast<std::streamsize>(block.bytes.size()));
    }
    return output.good();
}

// The index of the first function call block with this id, at or after the start index.
bool find_call(const CaptureFile& file, format::ApiCallId id, size_t start, size_t& index, std::string& error)
{
    for (size_t i = start; i < file.blocks.size(); ++i)
    {
        if (file.blocks[i].is_function_call() && file.blocks[i].api_call_id() == id)
        {
            index = i;
            return true;
        }
    }
    error = "the source has no function call block with id " + std::to_string(static_cast<uint32_t>(id));
    return false;
}

bool decode_call(const CaptureFile& file, const Block& block, DecodedCall& call, std::string& error)
{
    const uint8_t* bytes = block.bytes.data();
    if (block.is_compressed())
    {
        format::CompressedFunctionCallHeader header{};
        std::memcpy(&header, bytes, sizeof(header));
        call.api_call_id = header.api_call_id;
        call.thread_id   = header.thread_id;
        if (file.compressor == nullptr)
        {
            error = "the source has a compressed block and no compression option";
            return false;
        }
        call.parameters.resize(header.uncompressed_size);
        const size_t compressed_size = block.bytes.size() - sizeof(header);
        const size_t written         = file.compressor->Decompress(
            compressed_size, bytes + sizeof(header), header.uncompressed_size, call.parameters.data());
        if (written != header.uncompressed_size)
        {
            error = "could not decompress a block of the source";
            return false;
        }
    }
    else
    {
        format::FunctionCallHeader header{};
        std::memcpy(&header, bytes, sizeof(header));
        call.api_call_id = header.api_call_id;
        call.thread_id   = header.thread_id;
        call.parameters.assign(block.bytes.begin() + sizeof(header), block.bytes.end());
    }
    return true;
}

// The block for a call, uncompressed, so an edit to the parameters lands as it is.
Block encode_call(const DecodedCall& call)
{
    format::FunctionCallHeader header{};
    header.block_header.size = sizeof(header) - sizeof(format::BlockHeader) + call.parameters.size();
    header.block_header.type = format::BlockType::kFunctionCallBlock;
    header.api_call_id       = call.api_call_id;
    header.thread_id         = call.thread_id;

    Block block;
    block.bytes.resize(sizeof(header) + call.parameters.size());
    std::memcpy(block.bytes.data(), &header, sizeof(header));
    std::memcpy(block.bytes.data() + sizeof(header), call.parameters.data(), call.parameters.size());
    return block;
}

// Decode the first call with this id, hand the parameters to the edit, and write the block back.
template <typename Edit>
bool edit_call(CaptureFile& file, format::ApiCallId id, Edit edit, std::string& error)
{
    size_t index = 0;
    if (!find_call(file, id, 0, index, error))
    {
        return false;
    }
    DecodedCall call;
    if (!decode_call(file, file.blocks[index], call, error))
    {
        return false;
    }
    if (!edit(call, error))
    {
        return false;
    }
    file.blocks[index] = encode_call(call);
    return true;
}

bool apply_structure_mutation(CaptureMutation mutation, CaptureFile& file, std::string& error)
{
    if (file.blocks.size() <= kTargetBlock)
    {
        error = "the source has fewer than " + std::to_string(kTargetBlock + 1) + " blocks";
        return false;
    }
    switch (mutation)
    {
        case CaptureMutation::kTruncatedAtBlock:
            file.blocks.resize(kTargetBlock);
            break;
        case CaptureMutation::kTruncatedInBlock:
            file.blocks.resize(kTargetBlock + 1);
            file.blocks.back().bytes.resize(sizeof(format::BlockHeader) + 5);
            break;
        case CaptureMutation::kBadBlockSize:
        {
            const uint64_t huge_size = 0x00FFFFFFFFFFFFFFull;
            std::memcpy(file.blocks[kTargetBlock].bytes.data(), &huge_size, sizeof(huge_size));
            break;
        }
        case CaptureMutation::kBadMagic:
            std::memcpy(file.prefix.data(), "NOPE", 4);
            break;
        case CaptureMutation::kBadVersion:
        {
            const uint32_t unsupported_major_version = 99;
            std::memcpy(file.prefix.data() + offsetof(format::FileHeader, major_version),
                        &unsupported_major_version,
                        sizeof(unsupported_major_version));
            break;
        }
        case CaptureMutation::kBadCompressedPayload:
        {
            Block* compressed = nullptr;
            for (Block& block : file.blocks)
            {
                if (block.is_compressed() && block.header().size > 64)
                {
                    compressed = &block;
                    break;
                }
            }
            if (compressed == nullptr)
            {
                error = "the source has no compressed block with more than 64 bytes";
                return false;
            }
            // Leave the block header and the compressed call header alone and flip the bytes
            // after them, so the decompressor gets a well formed block with garbage inside.
            const size_t payload = sizeof(format::CompressedFunctionCallHeader);
            for (size_t i = payload + 4; i < payload + 40; ++i)
            {
                compressed->bytes[i] ^= 0xFF;
            }
            break;
        }
        default:
            error = "not a structure mutation";
            return false;
    }
    return true;
}

bool apply_content_mutation(CaptureMutation mutation, CaptureFile& file, std::string& error)
{
    using format::ApiCallId;
    switch (mutation)
    {
        case CaptureMutation::kHandleNeverCreated:
            // The first parameter of vkCmdDraw is the command buffer id.
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCmdDraw,
                [](DecodedCall& call, std::string&) {
                    const format::HandleId never_created = 0xDEADBEEF;
                    std::memcpy(call.parameters.data(), &never_created, sizeof(never_created));
                    return true;
                },
                error);
        case CaptureMutation::kSecondHandleNeverCreated:
            // The parameters of vkGetSwapchainImagesKHR start with the device id and the swapchain id.
            return edit_call(
                file,
                ApiCallId::ApiCall_vkGetSwapchainImagesKHR,
                [](DecodedCall& call, std::string&) {
                    const format::HandleId never_created = 0xDEADBEEF;
                    std::memcpy(
                        call.parameters.data() + sizeof(format::HandleId), &never_created, sizeof(never_created));
                    return true;
                },
                error);
        case CaptureMutation::kParameterBufferShortByOne:
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCmdDraw,
                [](DecodedCall& call, std::string&) {
                    call.parameters.pop_back();
                    return true;
                },
                error);
        case CaptureMutation::kParameterBufferShortByHalf:
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCmdDraw,
                [](DecodedCall& call, std::string&) {
                    call.parameters.resize(call.parameters.size() / 2);
                    return true;
                },
                error);
        case CaptureMutation::kCountBomb:
            // vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports). The array
            // is encoded as attributes, the address when kHasAddress is set, then the length.
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCmdSetViewport,
                [](DecodedCall& call, std::string& error) {
                    size_t   offset     = sizeof(format::HandleId) + 2 * sizeof(uint32_t);
                    uint32_t attributes = 0;
                    if (call.parameters.size() < offset + sizeof(attributes))
                    {
                        error = "the vkCmdSetViewport parameters are too short";
                        return false;
                    }
                    std::memcpy(&attributes, call.parameters.data() + offset, sizeof(attributes));
                    offset += sizeof(attributes);
                    if ((attributes & format::PointerAttributes::kHasAddress) != 0)
                    {
                        offset += sizeof(format::AddressEncodeType);
                    }
                    if ((attributes & format::PointerAttributes::kIsArray) == 0 ||
                        call.parameters.size() < offset + sizeof(format::SizeTEncodeType))
                    {
                        error = "the vkCmdSetViewport array is not where the encoding puts it";
                        return false;
                    }
                    const format::SizeTEncodeType bomb = 0x7fffffff;
                    std::memcpy(call.parameters.data() + offset, &bomb, sizeof(bomb));
                    return true;
                },
                error);
        case CaptureMutation::kUnknownApiCallId:
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCmdDraw,
                [](DecodedCall& call, std::string&) {
                    call.api_call_id = static_cast<ApiCallId>(format::MakeApiCallId(format::ApiFamily_Vulkan, 0xffff));
                    return true;
                },
                error);
        case CaptureMutation::kUnknownStructureType:
            // The triangle app puts a VkDebugUtilsMessengerCreateInfoEXT in the pNext chain of its
            // VkInstanceCreateInfo. Its sType is the only one of that value in the parameters.
            return edit_call(
                file,
                ApiCallId::ApiCall_vkCreateInstance,
                [](DecodedCall& call, std::string& error) {
                    const uint32_t known   = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    const uint32_t unknown = 0x7ffffff0;
                    for (size_t i = 0; i + sizeof(known) <= call.parameters.size(); ++i)
                    {
                        if (std::memcmp(call.parameters.data() + i, &known, sizeof(known)) == 0)
                        {
                            std::memcpy(call.parameters.data() + i, &unknown, sizeof(unknown));
                            return true;
                        }
                    }
                    error = "vkCreateInstance has no debug messenger in its pNext chain";
                    return false;
                },
                error);
        case CaptureMutation::kDrawBeforeBeginCommandBuffer:
        {
            size_t begin = 0;
            size_t draw  = 0;
            if (!find_call(file, ApiCallId::ApiCall_vkBeginCommandBuffer, 0, begin, error) ||
                !find_call(file, ApiCallId::ApiCall_vkCmdDraw, begin, draw, error))
            {
                return false;
            }
            Block moved = std::move(file.blocks[draw]);
            file.blocks.erase(file.blocks.begin() + draw);
            file.blocks.insert(file.blocks.begin() + begin, std::move(moved));
            return true;
        }
        case CaptureMutation::kAnnotationLabelPastBlock:
            for (Block& block : file.blocks)
            {
                if (block.base_type() == format::BlockType::kAnnotation)
                {
                    const uint32_t huge_label = 0x7fffffff;
                    std::memcpy(block.bytes.data() + offsetof(format::AnnotationHeader, label_length),
                                &huge_label,
                                sizeof(huge_label));
                    return true;
                }
            }
            error = "the source has no annotation block";
            return false;
        case CaptureMutation::kCompressionFlagOnUncompressed:
        {
            size_t index = 0;
            if (!find_call(file, ApiCallId::ApiCall_vkEndCommandBuffer, 0, index, error))
            {
                return false;
            }
            Block& block = file.blocks[index];
            if (block.is_compressed())
            {
                error = "the first vkEndCommandBuffer block is compressed already";
                return false;
            }
            const format::BlockType flagged = format::BlockType::kCompressedFunctionCallBlock;
            std::memcpy(block.bytes.data() + offsetof(format::BlockHeader, type), &flagged, sizeof(flagged));
            return true;
        }
        case CaptureMutation::kMissingStateSetup:
        {
            // The state setup of a trimmed capture sits between a begin and an end state marker.
            size_t begin = file.blocks.size();
            size_t end   = file.blocks.size();
            for (size_t i = 0; i < file.blocks.size(); ++i)
            {
                if (file.blocks[i].base_type() != format::BlockType::kStateMarkerBlock)
                {
                    continue;
                }
                format::Marker marker{};
                std::memcpy(&marker, file.blocks[i].bytes.data(), sizeof(marker));
                if (marker.marker_type == format::MarkerType::kBeginMarker && begin == file.blocks.size())
                {
                    begin = i;
                }
                else if (marker.marker_type == format::MarkerType::kEndMarker && begin != file.blocks.size())
                {
                    end = i;
                    break;
                }
            }
            if (end == file.blocks.size())
            {
                error = "the source has no state setup between state markers, so it is not a trimmed capture";
                return false;
            }
            file.blocks.erase(file.blocks.begin() + begin + 1, file.blocks.begin() + end);
            return true;
        }
        default:
            error = "not a content mutation";
            return false;
    }
}

} // namespace

const char* to_string(CaptureMutation mutation)
{
    for (const MutationName& entry : kNames)
    {
        if (entry.mutation == mutation)
        {
            return entry.name;
        }
    }
    return "unknown";
}

const std::vector<CaptureMutation>& all_capture_mutations()
{
    static const std::vector<CaptureMutation> all = [] {
        std::vector<CaptureMutation> result;
        for (const MutationName& entry : kNames)
        {
            result.push_back(entry.mutation);
        }
        return result;
    }();
    return all;
}

bool capture_mutation_from_string(const std::string& name, CaptureMutation& mutation)
{
    for (const MutationName& entry : kNames)
    {
        if (name == entry.name)
        {
            mutation = entry.mutation;
            return true;
        }
    }
    return false;
}

bool write_mutated_capture(CaptureMutation              mutation,
                           const std::filesystem::path& source,
                           const std::filesystem::path& destination,
                           std::string&                 error)
{
    CaptureFile file;
    if (!read_file(source, file, error))
    {
        return false;
    }
    const bool structure = mutation <= CaptureMutation::kBadCompressedPayload;
    if (!(structure ? apply_structure_mutation(mutation, file, error) : apply_content_mutation(mutation, file, error)))
    {
        return false;
    }
    return write_file(file, destination, error);
}
