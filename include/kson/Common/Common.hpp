#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <cassert>
#include <cmath>
#include <cstdint>
#include "kson/Encoding/Encoding.hpp"

namespace kson
{
	constexpr std::int32_t kNumBTLanes = 4;
	constexpr std::int32_t kNumFXLanes = 2;
	constexpr std::int32_t kNumLaserLanes = 2;

	constexpr std::size_t kNumBTLanesSZ = std::size_t{ kNumBTLanes };
	constexpr std::size_t kNumFXLanesSZ = std::size_t{ kNumFXLanes };
	constexpr std::size_t kNumLaserLanesSZ = std::size_t{ kNumLaserLanes };

	using Pulse = std::int64_t;
	using RelPulse = std::int64_t;

	constexpr Pulse kResolution = 240;
	constexpr Pulse kResolution4 = kResolution * 4;

	// Curve subdivision interval for pre-conversion
	constexpr Pulse kCurveSubdivisionInterval = kResolution / 16;

	// The difference between Pulse and RelPulse is only for annotation
	static_assert(std::is_same_v<Pulse, RelPulse>);

	template <typename T>
	using ByPulse = std::map<Pulse, T>;

	template <typename T>
	using BTLane = std::array<ByPulse<T>, kNumBTLanesSZ>;

	template <typename T>
	using FXLane = std::array<ByPulse<T>, kNumFXLanesSZ>;

	template <typename T>
	using LaserLane = std::array<ByPulse<T>, kNumLaserLanesSZ>;

	template <typename T>
	using ByPulseMulti = std::multimap<Pulse, T>;

	template <typename T>
	using ByRelPulse = std::map<RelPulse, T>;

	template <typename T>
	using ByRelPulseMulti = std::multimap<RelPulse, T>;

	template <typename T>
	using ByMeasureIdx = std::map<std::int64_t, T>;

	template <typename T>
	struct DefKeyValuePair
	{
		std::string name;
		T v;
	};

	struct GraphValue
	{
		double v = 0.0;
		double vf = 0.0;

		GraphValue() = default;

		/*implicit*/ GraphValue(double v)
			: v(v)
			, vf(v)
		{
		}

		GraphValue(double v, double vf)
			: v(v)
			, vf(vf)
		{
		}
	};

	struct GraphCurveValue
	{
		double a = 0.0; // x-coordinate of the curve control point (0.0-1.0)
		double b = 0.0; // y-coordinate of the curve control point (0.0-1.0)

		GraphCurveValue() = default;

		GraphCurveValue(double a, double b)
			: a(a)
			, b(b)
		{
		}

		// Returns true if this represents a linear interpolation (no curve)
		[[nodiscard]]
		bool isLinear() const
		{
			return a == b;
		}
	};

	struct GraphPoint
	{
		GraphValue v;
		GraphCurveValue curve; // Default {0.0, 0.0} means linear interpolation

		GraphPoint() = default;

		/*implicit*/ GraphPoint(double value)
			: v(value)
			, curve()
		{
		}

		GraphPoint(const GraphValue& v)
			: v(v)
			, curve()
		{
		}

		GraphPoint(const GraphValue& v, const GraphCurveValue& curve)
			: v(v)
			, curve(curve)
		{
		}
	};

	struct Interval
	{
		RelPulse length = 0;
	};

	using Graph = ByPulse<GraphPoint>;

	struct GraphSection
	{
		ByRelPulse<GraphPoint> v;
	};

	// Use std::map instead of std::unordered_map to ensure stable output order in saving
	template <typename T>
	using Dict = std::map<std::string, T>;

	template <typename T, typename U>
	auto ValueItrAt(const std::map<T, U>& map, T key)
	{
		auto itr = map.upper_bound(key);
		if (itr != map.begin())
		{
			--itr;
		}
		return itr;
	}

	template <typename T, typename U>
	[[nodiscard]]
	U ValueAtOrDefault(const std::map<T, U>& map, T key, const U& defaultValue)
	{
		const auto itr = ValueItrAt(map, key);
		if (itr == map.end() || key < itr->first)
		{
			return defaultValue;
		}
		return itr->second;
	}

	template <typename T>
	[[nodiscard]]
	std::size_t CountInRange(const ByPulse<T>& map, Pulse start, Pulse end)
	{
		static_assert(std::is_signed_v<Pulse>);
		assert(start <= end);

		const auto itr1 = map.upper_bound(start - Pulse{ 1 });
		if (itr1 == map.end() || itr1->first >= end)
		{
			return std::size_t{ 0 };
		}
		const auto itr2 = map.upper_bound(end - Pulse{ 1 });
		return static_cast<std::size_t>(std::distance(itr1, itr2));
	}

	template <typename T>
	[[nodiscard]]
	auto FirstInRange(const ByPulse<T>& map, Pulse start, Pulse end)
	{
		static_assert(std::is_signed_v<Pulse>);
		assert(start <= end);

		const auto itr = map.upper_bound(start - Pulse{ 1 });
		if (itr == map.end() || itr->first >= end)
		{
			return map.end();
		}
		return itr;
	}

	template <typename T>
	[[nodiscard]]
	auto IntervalAt(const ByPulse<T>& map, Pulse pulse)
	{
		const auto itr = ValueItrAt(map, pulse);
		if (itr != map.end() && itr->first <= pulse && pulse < itr->first + itr->second.length)
		{
			return itr;
		}
		return map.end();
	}

	[[nodiscard]]
	inline double RemoveFloatingPointError(double value)
	{
		// Round the value to eight decimal places (e.g. "0.700000004" -> "0.7")
		const double rounded = std::round(value * 1e8) / 1e8;

		// Return rounded only for almost exact values
		// (e.g. "0.700000001" -> "0.7",  "1.66666666667" -> "1.66666666667")
		if (std::abs(rounded - value) < 1e-9)
		{
			return rounded;
		}
		else
		{
			return value;
		}
	}

	[[nodiscard]]
	inline bool AlmostEquals(double a, double b)
	{
		return std::round(a * 1e8) == std::round(b * 1e8);
	}
}
