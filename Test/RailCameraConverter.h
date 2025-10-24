#pragma once

#include "Math/Transform.h"
#include "RailCameraData.h"

namespace RailCameraSystem {

	class RailCameraConverter {
	public:

		static Transform ConvertToLeftHand(const Transform& blenderTransform) {
			Transform gameTransform;

            gameTransform.translate.x = blenderTransform.translate.x;
            gameTransform.translate.y = blenderTransform.translate.z;
            gameTransform.translate.z = blenderTransform.translate.y;


            //blenderの値をQuaternion変換
            Quaternion qConv;
            qConv.w = blenderTransform.rotate.w;
            qConv.x = blenderTransform.rotate.x;
            qConv.y = -blenderTransform.rotate.z;
            qConv.z = -blenderTransform.rotate.y;

            // デフォルトの向きの差(Blender: -Z, Engine: +Z)を補正するための
            const float halfAngle = (-90.0f * Math::ToRadian) * 0.5f;

            Quaternion qFix;
            qFix.w = std::cos(halfAngle);
            qFix.x = std::sin(halfAngle);
            qFix.y = 0.0f;
            qFix.z = 0.0f;

            //2つの回転を合成する (qConvを適用した後、qFixを適用)
            gameTransform.rotate = qFix * qConv;
			return gameTransform;
		}

	};

}
