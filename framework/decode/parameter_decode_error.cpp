#include "decode/parameter_decode_error.h"

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)

namespace
{
thread_local bool        pending = false;
thread_local std::string pending_message;
thread_local size_t      chain_depth = 0;
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

ChainDepthGuard::ChainDepthGuard(const char* member_name)
{
    ++chain_depth;
    if (chain_depth > kMaxDepth)
    {
        too_deep_ = true;
        ParameterDecodeError::AddPendingMessage(std::string("a ") + member_name + " chain has more than " +
                                                std::to_string(kMaxDepth) + " structs");
    }
}

ChainDepthGuard::~ChainDepthGuard()
{
    --chain_depth;
}

GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)
