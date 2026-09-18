#include "capture_mutations.h"

#include "format/format.h"
#include "format/format_util.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

const char* to_string(CaptureMutation mutation)
{
    switch (mutation)
    {
        case CaptureMutation::kTruncatedAtBlock:
            return "truncated-at-block";
        case CaptureMutation::kTruncatedInBlock:
            return "truncated-in-block";
        case CaptureMutation::kBadBlockSize:
            return "bad-block-size";
        case CaptureMutation::kBadMagic:
            return "bad-magic";
        case CaptureMutation::kBadVersion:
            return "bad-version";
        case CaptureMutation::kBadCompressedPayload:
            return "bad-compressed-payload";
    }
    return "unknown";
}

namespace
{

struct Block
{
    size_t                        offset = 0; // Of the block header.
    gfxrecon::format::BlockHeader header{};
};

// The block that the mutations damage. Far enough in that the file header, the options and the
// first calls are intact, and early enough that a short file still holds it.
const size_t kTargetBlock = 40;

bool walk_blocks(const std::vector<uint8_t>& file, std::vector<Block>& blocks, std::string& error)
{
    using namespace gfxrecon::format;
    if (file.size() < sizeof(FileHeader))
    {
        error = "the file is shorter than a file header";
        return false;
    }
    FileHeader file_header{};
    std::memcpy(&file_header, file.data(), sizeof(file_header));
    if (file_header.fourcc != GFXRECON_FOURCC)
    {
        error = "the source is not a capture file";
        return false;
    }
    size_t offset = sizeof(FileHeader) + file_header.num_options * sizeof(FileOptionPair);
    while (offset + sizeof(BlockHeader) <= file.size())
    {
        Block block;
        block.offset = offset;
        std::memcpy(&block.header, file.data() + offset, sizeof(BlockHeader));
        blocks.push_back(block);
        offset += sizeof(BlockHeader) + block.header.size;
    }
    if (offset != file.size())
    {
        error = "the source does not end on a block boundary";
        return false;
    }
    if (blocks.size() <= kTargetBlock)
    {
        error = "the source has fewer than " + std::to_string(kTargetBlock + 1) + " blocks";
        return false;
    }
    return true;
}

} // namespace

bool write_mutated_capture(CaptureMutation              mutation,
                           const std::filesystem::path& source,
                           const std::filesystem::path& destination,
                           std::string&                 error)
{
    using namespace gfxrecon::format;

    std::ifstream input{ source, std::ios::binary };
    if (!input.is_open())
    {
        error = "could not open " + source.string();
        return false;
    }
    std::vector<uint8_t> file{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };

    std::vector<Block> blocks;
    if (!walk_blocks(file, blocks, error))
    {
        return false;
    }
    const Block& target = blocks[kTargetBlock];

    switch (mutation)
    {
        case CaptureMutation::kTruncatedAtBlock:
            file.resize(target.offset);
            break;
        case CaptureMutation::kTruncatedInBlock:
            file.resize(target.offset + sizeof(BlockHeader) + 5);
            break;
        case CaptureMutation::kBadBlockSize:
        {
            const uint64_t huge_size = 0x00FFFFFFFFFFFFFFull;
            std::memcpy(file.data() + target.offset, &huge_size, sizeof(huge_size));
            break;
        }
        case CaptureMutation::kBadMagic:
            std::memcpy(file.data(), "NOPE", 4);
            break;
        case CaptureMutation::kBadVersion:
        {
            const uint32_t unsupported_major_version = 99;
            std::memcpy(file.data() + offsetof(FileHeader, major_version),
                        &unsupported_major_version,
                        sizeof(unsupported_major_version));
            break;
        }
        case CaptureMutation::kBadCompressedPayload:
        {
            const Block* compressed = nullptr;
            for (const Block& block : blocks)
            {
                if (IsBlockCompressed(block.header.type) && block.header.size > 64)
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
            const size_t payload = compressed->offset + sizeof(CompressedFunctionCallHeader);
            for (size_t i = payload + 4; i < payload + 40; ++i)
            {
                file[i] ^= 0xFF;
            }
            break;
        }
    }

    std::ofstream output{ destination, std::ios::binary | std::ios::trunc };
    if (!output.is_open())
    {
        error = "could not write " + destination.string();
        return false;
    }
    output.write(reinterpret_cast<const char*>(file.data()), static_cast<std::streamsize>(file.size()));
    return output.good();
}
