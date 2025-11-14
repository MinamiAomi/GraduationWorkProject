///
/// エディターマネージャー
/// 

#pragma once

#include <memory>
#include <vector>

#include "Graphics/Core/DescriptorHandle.h"
#include "Graphics/Core/CommandContext.h"

namespace Editer {

    class EditerManager {
    public:
        EditerManager() {}

        void Initialize();
        void Render();
        void RenderToColorBuffer(CommandContext& commandContext);
        void Finalize();

    private:
        DescriptorHandle descriptor_;

    };

}