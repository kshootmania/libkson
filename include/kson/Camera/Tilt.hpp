#pragma once
#include "kson/Common/Common.hpp"
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

	// Tilt-specific GraphValue that allows vf to be either double or AutoTiltType
	struct TiltGraphValue
	{
		double v = 0.0;
		std::variant<double, AutoTiltType> vf = 0.0;

		TiltGraphValue() = default;

		/*implicit*/ TiltGraphValue(double v)
			: v(v)
			, vf(v)
		{
		}

		TiltGraphValue(double v, double vf)
			: v(v)
			, vf(vf)
		{
		}

		TiltGraphValue(double v, AutoTiltType vf)
			: v(v)
			, vf(vf)
		{
		}
	};

	// Tilt-specific GraphPoint
	struct TiltGraphPoint
	{
		TiltGraphValue v;
		GraphCurveValue curve; // Default {0.0, 0.0} means linear interpolation

		TiltGraphPoint() = default;

		/*implicit*/ TiltGraphPoint(double value)
			: v(value)
			, curve()
		{
		}

		/*implicit*/ TiltGraphPoint(TiltGraphValue v)
			: v(v)
			, curve()
		{
		}

		TiltGraphPoint(TiltGraphValue v, GraphCurveValue curve)
			: v(v)
			, curve(curve)
		{
		}
	};

	using TiltValue = std::variant<AutoTiltType, TiltGraphPoint>;

	// Get scale value from AutoTiltType
	[[nodiscard]]
	double GetAutoTiltScale(AutoTiltType type);

	// Check if AutoTiltType is a keep type
	[[nodiscard]]
	bool IsKeepAutoTiltType(AutoTiltType type);
}
