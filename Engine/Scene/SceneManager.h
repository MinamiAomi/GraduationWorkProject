#pragma once

#include <memory>

#include "SceneTransition.h"

class BaseScene;

class SceneManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得
    /// </summary>
    /// <returns></returns>
    static SceneManager* GetInstance();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// シーンチェンジ
    /// </summary>
    /// <typeparam name="T">次のシーンのクラス</typeparam>
    /// <param name="useTransition">画面遷移を使用する</param>
    template<class T>
    void ChangeScene(bool useTransition = true) {
        static_assert(std::is_base_of<BaseScene, T>::value, "BaseSceneを継承していません。");
        nextScene_ = std::make_unique<T>();
        if (useTransition) {
            sceneTransition_.Start(SceneTransition::Mode::In);
        }
    }

    // ゲッター

    SceneTransition& GetSceneTransition() { return sceneTransition_; }

private:
    SceneManager();
    ~SceneManager();
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<BaseScene> currentScene_;
    std::unique_ptr<BaseScene> nextScene_;
    SceneTransition sceneTransition_;

};