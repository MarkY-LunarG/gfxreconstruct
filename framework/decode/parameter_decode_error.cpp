#include "decode/parameter_decode_error.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

namespace
{
thread_local bool        pending = false;
thread_local std::string pending_message;
} // namespace

void ParameterDecodeError::AddPendingMessage(const std::string& message)
{
    if (!pending)
    {
        pending         = true;
        pending_message = message;
    }
}

bool ParameterDecodeError::Pending()
{
    return pending;
}

bool ParameterDecodeError::GetPendingMessage(std::string* message)
{
    if (!pending)
    {
        return false;
    }
    if (message != nullptr)
    {
        *message = std::move(pending_message);
    }
    pending = false;
    pending_message.clear();
    return true;
}

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)
