#include "kson/Compat/CompatInfo.hpp"
#include <cstdlib>

bool kson::CompatInfo::isKshVersionOlderThan(int kshVersionInt) const
{
	if (kshVersion.empty())
	{
		// This chart data is not converted from KSH older than ver=200
		return false;
	}

	const int chartKshVersionInt = std::atoi(kshVersion.c_str());
	return 100 <= chartKshVersionInt && chartKshVersionInt < kshVersionInt;
}
