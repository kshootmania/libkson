#include "kson/Util/TiltUtils.hpp"
#include "kson/Util/GraphCurve.hpp"
#include <variant>

std::optional<double> kson::ManualTiltValueAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse)
{
	if (tiltValue.empty())
	{
		return std::nullopt;
	}

	const auto currentIt = ValueItrAt(tiltValue, currentPulse);
	if (currentIt == tiltValue.end() || currentPulse < currentIt->first)
	{
		return std::nullopt;
	}

	const Pulse currentValuePulse = currentIt->first;
	const TiltValue& currentValue = currentIt->second;

	if (!std::holds_alternative<TiltGraphPoint>(currentValue))
	{
		return std::nullopt;
	}

	const TiltGraphPoint& currentPoint = std::get<TiltGraphPoint>(currentValue);

	// If vf is AutoTiltType, return nullopt (this is not manual tilt)
	if (std::holds_alternative<AutoTiltType>(currentPoint.v.vf))
	{
		return std::nullopt;
	}

	const double currentVf = std::get<double>(currentPoint.v.vf);

	auto nextIt = currentIt;
	++nextIt;

	if (nextIt != tiltValue.end() && std::holds_alternative<TiltGraphPoint>(nextIt->second))
	{
		// Next is also manual tilt: interpolate between vf and next v
		const TiltGraphPoint& nextPoint = std::get<TiltGraphPoint>(nextIt->second);
		const Pulse nextPulse = nextIt->first;
		const Pulse segmentLength = nextPulse - currentValuePulse;
		if (segmentLength <= 0)
		{
			return nextPoint.v.v;
		}

		const double lerpRate = static_cast<double>(currentPulse - currentValuePulse) / static_cast<double>(segmentLength);

		// Apply curve if present
		const double curveValue = EvaluateCurve(currentPoint.curve, lerpRate);

		return std::lerp(currentVf, nextPoint.v.v, curveValue);
	}
	else
	{
		// Next is auto tilt or no next: continue with vf value
		return currentVf;
	}
}

double kson::AutoTiltScaleAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse)
{
	if (tiltValue.empty())
	{
		return 1.0;
	}

	const auto it = ValueItrAt(tiltValue, currentPulse);
	if (it != tiltValue.end() && currentPulse >= it->first)
	{
		if (std::holds_alternative<AutoTiltType>(it->second))
		{
			const AutoTiltType type = std::get<AutoTiltType>(it->second);
			return GetAutoTiltScale(type);
		}
	}
	return 1.0;
}

bool kson::AutoTiltKeepAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse)
{
	if (tiltValue.empty())
	{
		return false;
	}

	const auto it = ValueItrAt(tiltValue, currentPulse);
	if (it != tiltValue.end() && currentPulse >= it->first)
	{
		if (std::holds_alternative<AutoTiltType>(it->second))
		{
			const AutoTiltType type = std::get<AutoTiltType>(it->second);
			return IsKeepAutoTiltType(type);
		}
	}
	return false;
}
