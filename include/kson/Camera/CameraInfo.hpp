#pragma once
#include "kson/Common/Common.hpp"
#include "Cam.hpp"
#include "Tilt.hpp"

namespace kson
{
	struct CameraInfo
	{
		CamInfo cam;
		ByPulse<TiltValue> tilt;
	};
}
