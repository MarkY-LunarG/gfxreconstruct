#ifndef GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H
#define GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H

#include "util/defines.h"

#include <string>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

// A parameter buffer that did not hold what a decoder needed: a value past the end of the
// buffer, an array length that cannot fit, or a structure the decoder does not know. The value
// and pointer decoders report it here, because their return value is a byte count that the
// generated decoders add up and never test. The dispatch visitor takes the report after each
// block and fails the block with its index, so a call is never processed with parameters that
// were never decoded. The state is per thread, and one block is decoded on one thread.
class ParameterDecodeError
{
  public:
    // Keep the first report of the block. Later reports of the same block add nothing.
    static void Report(const std::string& message);

    // True when a report is pending and nothing has taken it. A decoder that sees this decodes
    // nothing more, so no allocation and no consumer call follows a corrupt parameter.
    static bool Pending();

    // True when a report is pending. Moves its message out and clears the state.
    static bool Take(std::string* message);
};

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)

#endif // GFXRECON_DECODE_PARAMETER_DECODE_ERROR_H
