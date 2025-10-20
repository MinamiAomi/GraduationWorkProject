#include "Animation.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Debug/Debug.h"

namespace {

    AnimationSet ParseAnimation(const aiAnimation* animation) {
        AnimationSet result;
        // 時間の単位を秒に変換
        result.duration = float(animation->mDuration / animation->mTicksPerSecond);

        for (uint32_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
            auto srcNodeAnimation = animation->mChannels[channelIndex];
            auto& destNodeAnimation = result.nodeAnimations[srcNodeAnimation->mNodeName.C_Str()];

            destNodeAnimation.translate.keyframes.resize(srcNodeAnimation->mNumPositionKeys);
            for (uint32_t keyIndex = 0; keyIndex < srcNodeAnimation->mNumPositionKeys; ++keyIndex) {
                auto& srcKey = srcNodeAnimation->mPositionKeys[keyIndex];
                auto& destKey = destNodeAnimation.translate.keyframes[keyIndex];
                destKey.time = float(srcKey.mTime / animation->mDuration);
                destKey.value = { srcKey.mValue.x, srcKey.mValue.y, srcKey.mValue.z };
                // 右手→左手
                destKey.value.z *= -1.0f;
            }

            destNodeAnimation.rotate.keyframes.resize(srcNodeAnimation->mNumRotationKeys);
            for (uint32_t keyIndex = 0; keyIndex < srcNodeAnimation->mNumRotationKeys; ++keyIndex) {
                auto& srcKey = srcNodeAnimation->mRotationKeys[keyIndex];
                auto& destKey = destNodeAnimation.rotate.keyframes[keyIndex];
                destKey.time = float(srcKey.mTime / animation->mDuration);
                destKey.value = Quaternion{ srcKey.mValue.x, srcKey.mValue.y, srcKey.mValue.z, srcKey.mValue.w };
                // 右手→左手
                destKey.value.x *= -1.0f;
                destKey.value.y *= -1.0f;
                //destKey.value.z *= -1.0f;
            }

            destNodeAnimation.scale.keyframes.resize(srcNodeAnimation->mNumScalingKeys);
            for (uint32_t keyIndex = 0; keyIndex < srcNodeAnimation->mNumScalingKeys; ++keyIndex) {
                auto& srcKey = srcNodeAnimation->mScalingKeys[keyIndex];
                auto& destKey = destNodeAnimation.scale.keyframes[keyIndex];
                destKey.time = float(srcKey.mTime / animation->mDuration);
                destKey.value = { srcKey.mValue.x, srcKey.mValue.y, srcKey.mValue.z };
            }
        }

        return result;
    }



}

std::shared_ptr<Animation> Animation::Load(const std::filesystem::path& path) {

    // privateコンストラクタをmake_sharedで呼ぶためのヘルパー
    struct Helper : Animation {
        Helper() : Animation() {}
    };
    std::shared_ptr<Animation> animation = std::make_shared<Helper>();

    auto directory = path.parent_path();

    Assimp::Importer importer;
    int flags = 0;
    const aiScene* scene = importer.ReadFile(path.string(), flags);
    // 読み込めた
    if (!scene) {
        Debug::Log(importer.GetErrorString());
        assert(false);
    }
    // アニメーションがある
    assert(scene->mNumAnimations != 0);
    // 配列を確保しておく
    for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
        aiString name = scene->mAnimations[animationIndex]->mName;
        animation->animationSet_[name.C_Str()] = ParseAnimation(scene->mAnimations[animationIndex]);
    }

    return animation;
}

Vector3 CalculateValue(const AnimationCurve<Vector3>& animationCurve, float time) {
    assert(!animationCurve.keyframes.empty());
    if (animationCurve.keyframes.size() == 1 || time <= animationCurve.keyframes[0].time) {
        return animationCurve.keyframes[0].value;
    }

    for (size_t index = 0; index < animationCurve.keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (animationCurve.keyframes[index].time <= time && time <= animationCurve.keyframes[nextIndex].time) {
            float t = (time - animationCurve.keyframes[index].time) / (animationCurve.keyframes[nextIndex].time - animationCurve.keyframes[index].time);
            return Vector3::Lerp(t, animationCurve.keyframes[index].value, animationCurve.keyframes[nextIndex].value);
        }
    }
    return animationCurve.keyframes.rbegin()->value;
}

Quaternion CalculateValue(const AnimationCurve<Quaternion>& animationCurve, float time) {
    assert(!animationCurve.keyframes.empty());
    if (animationCurve.keyframes.size() == 1 || time <= animationCurve.keyframes[0].time) {
        return animationCurve.keyframes[0].value;
    }

    for (size_t index = 0; index < animationCurve.keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (animationCurve.keyframes[index].time <= time && time <= animationCurve.keyframes[nextIndex].time) {
            float t = (time - animationCurve.keyframes[index].time) / (animationCurve.keyframes[nextIndex].time - animationCurve.keyframes[index].time);
            return Quaternion::Slerp(t, animationCurve.keyframes[index].value, animationCurve.keyframes[nextIndex].value);
        }
    }
    return animationCurve.keyframes.rbegin()->value;
}
