#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct TimeSig
	{
		std::int32_t n = 4;
		std::int32_t d = 4;
	};

	// Returns whether the time signature has a positive measure length in pulses
	[[nodiscard]]
	constexpr bool IsValidTimeSig(const TimeSig& timeSig)
	{
		return timeSig.n >= 1
			&& timeSig.d >= 1
			&& kResolution4 * timeSig.n >= timeSig.d;
	}
}
