#pragma once

#include "Math/Transform.h"
#include "RailData.h"

namespace RailSystem {

	class RailConverter {
	public:

		static Transform ConvertToLeftHand(const Transform& blenderTransform) {
			Transform gameTransform;

            gameTransform.translate.x = blenderTransform.translate.x;
            gameTransform.translate.y = blenderTransform.translate.z;
            gameTransform.translate.z = blenderTransform.translate.y;



            gameTransform.rotate.w = blenderTransform.rotate.w;
            gameTransform.rotate.x = -blenderTransform.rotate.x;
            gameTransform.rotate.y = -blenderTransform.rotate.z;
            gameTransform.rotate.z = -blenderTransform.rotate.y;
			
			Quaternion qConv;
			qConv.w = blenderTransform.rotate.w;
			qConv.x = -blenderTransform.rotate.x; 
			qConv.y = -blenderTransform.rotate.z; 
			qConv.z = -blenderTransform.rotate.y;

			const float angle = 90.0f * Math::ToRadian;

			Quaternion qFix;
			qFix.w = std::cos(angle * 0.5f);
			qFix.x = std::sin(angle * 0.5f);
			qFix.y = 0.0f;
			qFix.z = 0.0f;

			gameTransform.rotate = qConv * qFix;
			return gameTransform;
		}

	};

}
