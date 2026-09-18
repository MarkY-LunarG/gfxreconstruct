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
// with parameters that were never decoded. The state is per thread, and one block is decoded
// on one thread.
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

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H
