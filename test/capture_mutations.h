
#ifndef GFXRECONSTRUCT_CAPTURE_MUTATIONS_H
#define GFXRECONSTRUCT_CAPTURE_MUTATIONS_H

#include <filesystem>
#include <string>

// Ways to damage a capture file. The tools must refuse each one with a message and must not
// crash, or, for a file that ends early, must process what is there and say so.
enum class CaptureMutation
{
    kTruncatedAtBlock,     // The file ends on a block boundary. A clean end is not detectable.
    kTruncatedInBlock,     // The file ends inside a block.
    kBadBlockSize,         // A block header claims a size far past the end of the file.
    kBadMagic,             // The four character code in the file header is wrong.
    kBadVersion,           // The file header claims a major version that no tool supports.
    kBadCompressedPayload, // The payload of a compressed block is garbage.
};

const char* to_string(CaptureMutation mutation);

// Read a good capture, apply the mutation, and write the result. The mutation happens near the
// front of the file, after the first frames of setup, so every tool reaches it. Returns false
// with the reason when the source is not a capture this function can walk.
bool write_mutated_capture(CaptureMutation              mutation,
                           const std::filesystem::path& source,
                           const std::filesystem::path& destination,
                           std::string&                 error);

#endif // GFXRECONSTRUCT_CAPTURE_MUTATIONS_H
