#pragma once

#include "Math/MathUtils.h"

namespace SceneObjectSystem {
    //RailCameraConverterを参考に
    class SceneObjectConverter {
    public:

        static Vector3 ConvertTranslateToLeftHand(const Vector3& blenderTranslate) {
            Vector3 gameTranslate;

            gameTranslate.x = blenderTranslate.x;
            gameTranslate.y = blenderTranslate.z;
            gameTranslate.z = blenderTranslate.y;

            return gameTranslate;
        }

        static Quaternion ConvertRotateToLeftHand(const Quaternion& blenderRotate) {
            
            Quaternion gameQuaternion;
            gameQuaternion.w = blenderRotate.w;
            gameQuaternion.x = blenderRotate.x;
            gameQuaternion.y = -blenderRotate.z;
            gameQuaternion.z = -blenderRotate.y;

            return gameQuaternion;
        }

        static Vector3 ConvertSizeToLeftHand(const Vector3& blenderSize) {

            Vector3 gameSize;
            gameSize.x = blenderSize.x;
            gameSize.y = blenderSize.z;
            gameSize.z = blenderSize.y;

            return gameSize;
        }

    };
}
