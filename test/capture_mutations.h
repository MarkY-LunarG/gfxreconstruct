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

#ifndef GFXRECONSTRUCT_CAPTURE_MUTATIONS_H
#define GFXRECONSTRUCT_CAPTURE_MUTATIONS_H

#include <filesystem>
#include <string>
#include <vector>

// Ways to damage a capture file. The first group breaks the structure of the file. The tools
// must refuse each one with a message and must not crash, or, for a file that ends early, must
// process what is there and say so. The second group keeps the structure valid and makes the
// content wrong. The tools must report what they can see and must not crash, read past a
// buffer, or allocate what a count field claims.
enum class CaptureMutation
{
    // Structure.
    kTruncatedAtBlock,     // The file ends on a block boundary. A clean end is not detectable.
    kTruncatedInBlock,     // The file ends inside a block.
    kBadBlockSize,         // A block header claims a size far past the end of the file.
    kBadMagic,             // The four character code in the file header is wrong.
    kBadVersion,           // The file header claims a major version that no tool supports.
    kBadCompressedPayload, // The payload of a compressed block is garbage.

    // Content. The block that each one edits is written back uncompressed, so the edit is exact.
    kHandleNeverCreated,            // The command buffer of the first vkCmdDraw is an id that no call created.
    kSecondHandleNeverCreated,      // The swapchain of the first vkGetSwapchainImagesKHR is an id that no call created.
    kParameterBufferShortByOne,     // The parameter buffer of the first vkCmdDraw loses its last byte.
    kParameterBufferShortByHalf,    // The parameter buffer of the first vkCmdDraw loses its second half.
    kCountBomb,                     // The viewport array of the first vkCmdSetViewport claims 0x7fffffff elements.
    kUnknownApiCallId,              // The first vkCmdDraw carries an ApiCallId that this build does not know.
    kUnknownStructureType,          // The debug messenger in the pNext chain of vkCreateInstance has an unknown sType.
    kDrawBeforeBeginCommandBuffer,  // The first vkCmdDraw moves before the first vkBeginCommandBuffer.
    kAnnotationLabelPastBlock,      // The label of the first annotation block claims 0x7fffffff bytes.
    kCompressionFlagOnUncompressed, // The first vkEndCommandBuffer, an uncompressed block, gets the compressed bit.
    kMissingStateSetup,             // The blocks between the state markers of a trimmed capture are gone.
};

const char* to_string(CaptureMutation mutation);

// Every mutation, in the order of the enum. For a tool that lists them.
const std::vector<CaptureMutation>& all_capture_mutations();

// The mutation with this name, as to_string gives it. Returns false for an unknown name.
bool capture_mutation_from_string(const std::string& name, CaptureMutation& mutation);

// Read a good capture, apply the mutation, and write the result. Returns false with the reason
// when the source is not a capture this function can walk or lacks the block that the mutation
// needs. kMissingStateSetup needs a trimmed capture. The others take any capture with a frame of
// the triangle app.
bool write_mutated_capture(CaptureMutation              mutation,
                           const std::filesystem::path& source,
                           const std::filesystem::path& destination,
                           std::string&                 error);

#endif // GFXRECONSTRUCT_CAPTURE_MUTATIONS_H
