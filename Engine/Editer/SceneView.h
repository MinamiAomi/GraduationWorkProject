///
/// シーンビュー
/// 

#pragma once
#include "EditerView.h"

class CommandContext;

namespace Editer {

    class SceneView :
        public View {
    public:
        using View::View;

        void Render(CommandContext& commandContext);

        void SetUseMainImage(bool useMainImage) { useMainImage_ = useMainImage; }
        bool SetUseMainImage() const { return useMainImage_; }

    private:
        bool useMainImage_ = true;

    };

}