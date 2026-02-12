#pragma once
#include "kson/Common/Common.hpp"
#include "TimeSig.hpp"

namespace kson
{
	struct BeatInfo
	{
		ByPulse<double> bpm;

		ByMeasureIdx<TimeSig> timeSig;

		Graph scrollSpeed;

		ByPulse<RelPulse> stop;
	};
}
