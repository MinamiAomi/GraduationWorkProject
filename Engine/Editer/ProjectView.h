///
/// プロジェクトビュー
///

#pragma once
#include "EditerView.h"

namespace Editer {

    class ProjectView : 
        public View {
    public:
        using View::View;

        void Render();

    private:
        void RenderLeftWindow();
        void RenderRightWindow();

        enum AssetType {
            Texture     = 1 << 0,
            Model       = 1 << 1,
            Material    = 1 << 2,
            Animation   = 1 << 3,
            Sound       = 1 << 4,
            All         = Texture | Model| Material | Animation | Sound
        };

        AssetType showAssetTypes_ = All;
        float leftWindowSize_ = 100.0f;
        float itemScale_ = 1.0f;
    };

}