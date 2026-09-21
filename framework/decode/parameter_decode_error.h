#ifndef GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H
#define GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H

#include "util/defines.h"

#include <string>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

// A parameter buffer that did not hold what a decoder needed: a value past the end of the
// buffer, an array length that cannot fit, or a structure the decoder does not know. The value
// and pointer decoders add a pending message here, because their return value is a byte count
// that the generated decoders add up and never test. The dispatch visitor gets the pending
// message after each block and fails the block with its index, so a call is never processed
// with parameters that were never decoded. The generated replay consumers add a message for the
// same reason when the object that selects a call's dispatch table is not in the object table.
// The state is per thread, and one block is decoded on one thread.
class ParameterDecodeError
{
  public:
    // Keep the first message of the block. Later messages of the same block add nothing.
    static void AddPendingMessage(const std::string& message);

    // True when a message is pending and nothing has taken it. A decoder that sees this decodes
    // nothing more, so no allocation and no consumer call follows a corrupt parameter.
    static bool Pending();

    // True when a message is pending. Moves the message out and clears the state.
    static bool GetPendingMessage(std::string* message);
};

// The extension chain decoders use one recursion level per struct in a chain. This guard counts
// the depth on the thread. A chain deeper than the bound is not one that a driver accepted, and
// its decode would exhaust the stack, so the constructor adds the pending message and the decoder
// returns. The bound is far above any chain an application builds. The state is per thread.
class ChainDepthGuard
{
  public:
    static constexpr size_t kMaxDepth = 1024;

    // member_name is the chain member, "pNext" or "next", for the message.
    explicit ChainDepthGuard(const char* member_name);
    ~ChainDepthGuard();

    ChainDepthGuard(const ChainDepthGuard&)            = delete;
    ChainDepthGuard& operator=(const ChainDepthGuard&) = delete;

    // True when this level is past the bound. The constructor has added the message.
    bool TooDeep() const { return too_deep_; }

  private:
    bool too_deep_{ false };
};

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H
