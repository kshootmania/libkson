#pragma once
#include "kson/common/common.hpp"
#include <variant>

namespace kson
{
	enum class AutoTiltType
	{
		kNormal,
		kBigger,
		kBiggest,
		kKeepNormal,
		kKeepBigger,
		kKeepBiggest,
		kZero,
	};

	using TiltValue = std::variant<AutoTiltType, GraphPoint>;

	// Get scale value from AutoTiltType
	double GetAutoTiltScale(AutoTiltType type);

	// Check if AutoTiltType is a keep type
	bool IsKeepAutoTiltType(AutoTiltType type);
}
