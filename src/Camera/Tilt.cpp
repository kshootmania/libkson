#include "kson/Camera/Tilt.hpp"

[[nodiscard]]
double kson::GetAutoTiltScale(AutoTiltType type) noexcept
{
	switch (type)
	{
	case AutoTiltType::kZero:
		return 0.0;
	case AutoTiltType::kNormal:
	case AutoTiltType::kKeepNormal:
		return 1.0;
	case AutoTiltType::kBigger:
	case AutoTiltType::kKeepBigger:
		return 1.75;
	case AutoTiltType::kBiggest:
	case AutoTiltType::kKeepBiggest:
		return 2.5;
	default:
		return 1.0;
	}
}

[[nodiscard]]
bool kson::IsKeepAutoTiltType(AutoTiltType type) noexcept
{
	return type == AutoTiltType::kKeepNormal ||
		type == AutoTiltType::kKeepBigger ||
		type == AutoTiltType::kKeepBiggest;
}
