///
/// エディターのビューの基底クラス
/// 

#pragma once

namespace Editer {

    class EditerManager;
    class View {
    public:
        View(EditerManager& editerManager) : owner(editerManager), isDisplayed(true) {}
        virtual ~View() = 0{}

        EditerManager& owner;
        bool isDisplayed;
    };

}