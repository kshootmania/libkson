#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct KSHUnknownInfo
	{
		std::unordered_map<std::string, std::string> meta;
		std::unordered_map<std::string, ByPulseMulti<std::string>> option;
		ByPulseMulti<std::string> line;
	};

	struct CompatInfo
	{
		std::string kshVersion;
		KSHUnknownInfo kshUnknown;

		[[nodiscard]]
		bool isKSHVersionOlderThan(int kshVersionInt) const;
	};
}
