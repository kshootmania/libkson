#pragma once
#include <tuple>
#include "kson/common/common.hpp"

namespace kson
{
	struct CamGraphs
	{
		Graph zoomBottom; // "zoom_bottom"
		Graph zoomSide; // "zoom_side"
		Graph zoomTop; // "zoom_top"
		Graph rotationDeg; // Rotation degree
		Graph centerSplit; // "center_split"
	};

	struct CamPatternInvokeSwingValue
	{
		double scale = 250.0;
		std::int32_t repeat = 1;
		std::int32_t decayOrder = 0;
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

	using CamPatternInvokeSpin = detail::BasicCamPatternInvoke<std::tuple<>>; // Note: std::tuple<> is empty
	using CamPatternInvokeSwing = detail::BasicCamPatternInvoke<CamPatternInvokeSwingValue>;

	struct CamPatternLaserInvokeList
	{
		ByPulse<CamPatternInvokeSpin> spin;
		ByPulse<CamPatternInvokeSpin> halfSpin;
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
