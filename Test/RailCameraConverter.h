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


            Quaternion qConv;
            qConv.w = blenderTransform.rotate.w;
            qConv.x = blenderTransform.rotate.x;
            qConv.y = -blenderTransform.rotate.z;
            qConv.z = -blenderTransform.rotate.y;

            const float halfAngle = (-90.0f * Math::ToRadian) * 0.5f;

            Quaternion qFix;
            qFix.w = std::cos(halfAngle);
            qFix.x = std::sin(halfAngle);
            qFix.y = 0.0f;
            qFix.z = 0.0f;

            gameTransform.rotate = qFix * qConv;
			return gameTransform;
		}

	};

}
