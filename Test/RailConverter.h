#pragma once

#include "Math/Transform.h"
#include "RailData.h"

namespace RailSystem {

	class RailConverter {
	public:

		static Quaternion ConvertToLeftHand(const Quaternion& blenderTransform) {
			Quaternion result;

			result.w = blenderTransform.w;
			result.x = -blenderTransform.x;
			result.y = -blenderTransform.z;
			result.z = -blenderTransform.y;

			const float angle = 90.0f * Math::ToRadian;

			Quaternion qFix;
			qFix.w = std::cos(angle * 0.5f);
			qFix.x = std::sin(angle * 0.5f);
			qFix.y = 0.0f;
			qFix.z = 0.0f;

			return result * qFix;
		}

		static Vector3 ConvertToLeftHand(const Vector3& blenderTransform) {
			Vector3 result;

			result.x = blenderTransform.x;
			result.y = blenderTransform.z;
			result.z = blenderTransform.y;

			return result;
		}

		static Transform ConvertToLeftHand(const Transform& blenderTransform) {
			Transform gameTransform;
			gameTransform.translate = RailConverter::ConvertToLeftHand(blenderTransform.translate);
			gameTransform.rotate = RailConverter::ConvertToLeftHand(blenderTransform.rotate);
			return gameTransform;
		}


	};

}
