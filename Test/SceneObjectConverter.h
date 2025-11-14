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
            
            Quaternion qConv, gameQuaternion;
            qConv.w = blenderRotate.w;
            qConv.x = blenderRotate.x;
            qConv.y = -blenderRotate.z;
            qConv.z = -blenderRotate.y;

            const float halfAngle = (-90.0f * Math::ToRadian) * 0.5f;

            Quaternion qFix;
            qFix.w = std::cos(halfAngle);
            qFix.x = std::sin(halfAngle);
            qFix.y = 0.0f;
            qFix.z = 0.0f;

            gameQuaternion= qFix * qConv;
            return gameQuaternion;
        }

    };
}
