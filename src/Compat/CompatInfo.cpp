#include "kson/Compat/CompatInfo.hpp"
#include <cstdlib>

bool kson::CompatInfo::isKshVersionOlderThan(int kshVersionInt) const
{
	if (kshVersion.empty())
	{
		// This chart data is not converted from the KSH format
		return false;
	}

	const int chartKshVersionInt = std::atoi(kshVersion.c_str());
	return 100 <= chartKshVersionInt && chartKshVersionInt < kshVersionInt;
}
