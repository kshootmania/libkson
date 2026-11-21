#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct CamGraphs
	{
		Graph zoomBottom; // "zoom_bottom"
		Graph zoomSide; // "zoom_side"
		Graph zoomTop; // "zoom_top"
		Graph rotationDeg; // Rotation degree
		Graph rotationDegHighway; // "rotation_deg_highway"
		Graph centerSplit; // "center_split"
		Graph highwayWipe; // "highway_wipe"
	};

	struct CamPatternInvokeSpinValue
	{
		std::int32_t count = 1;
	};

	struct CamPatternInvokeHalfSpinValue
	{
		double deg = 60.0;
	};

	struct CamPatternInvokeSwingValue
	{
		double scale = 250.0;
		std::int32_t repeat = 3;
		std::int32_t decayOrder = 2;
	};

	namespace detail
	{
		template <typename ValueType>
		struct BasicCamPatternInvoke
		{
			std::int32_t d = 0; // laser slam direction, -1 (left) or 1 (right)
			RelPulse length = 0;
			ValueType v = {};
		};
	}

	using CamPatternInvokeSpin = detail::BasicCamPatternInvoke<CamPatternInvokeSpinValue>;
	using CamPatternInvokeHalfSpin = detail::BasicCamPatternInvoke<CamPatternInvokeHalfSpinValue>;
	using CamPatternInvokeSwing = detail::BasicCamPatternInvoke<CamPatternInvokeSwingValue>;

	struct CamPatternLaserInvokeList
	{
		ByPulse<CamPatternInvokeSpin> spin;
		ByPulse<CamPatternInvokeHalfSpin> halfSpin;
		ByPulse<CamPatternInvokeSwing> swing;
	};

	struct CamPatternLaserInfo
	{
		CamPatternLaserInvokeList slamEvent;
	};

	struct CamPatternInfo
	{
		CamPatternLaserInfo laser;
	};

	struct CamInfo
	{
		CamGraphs body;
		CamPatternInfo pattern;
	};
}
