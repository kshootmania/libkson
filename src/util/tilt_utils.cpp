#include "kson/util/tilt_utils.hpp"
#include "kson/util/graph_curve.hpp"
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

	if (!std::holds_alternative<GraphPoint>(currentValue))
	{
		return std::nullopt;
	}

	const GraphPoint& currentPoint = std::get<GraphPoint>(currentValue);

	auto nextIt = currentIt;
	++nextIt;

	if (nextIt != tiltValue.end() && std::holds_alternative<GraphPoint>(nextIt->second))
	{
		// Next is also manual tilt: interpolate between vf and next v
		const GraphPoint& nextPoint = std::get<GraphPoint>(nextIt->second);
		const Pulse nextPulse = nextIt->first;
		const double lerpRate = static_cast<double>(currentPulse - currentValuePulse) / static_cast<double>(nextPulse - currentValuePulse);

		// Apply curve if present
		const double curveValue = EvaluateCurve(currentPoint.curve, lerpRate);

		return std::lerp(currentPoint.v.vf, nextPoint.v.v, curveValue);
	}
	else
	{
		// Next is auto tilt or no next: continue with vf value
		return currentPoint.v.vf;
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
