#include "nmd_common.hpp"

namespace nmd
{
    static Context g_context;

    Context& GetContext() { return g_context; }

    void Begin()
    {
        g_context.drawList.vertices.clear();
        g_context.drawList.indices.clear();
        g_context.drawList.drawCommands.clear();
    }

    void End()
    {
        g_context.drawList.PushRemainingDrawCommands();
    }

} // namespace nmd
