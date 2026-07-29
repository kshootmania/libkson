#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	// KSH format version representing charts converted from kson
	// (CompatInfo::kshVersion is left empty when loading such charts)
	constexpr std::int32_t kKshVersionKsonBased = 200;

	struct KshUnknownInfo
	{
		std::unordered_map<std::string, std::string> meta;
		std::unordered_map<std::string, ByPulseMulti<std::string>> option;
		ByPulseMulti<std::string> line;

		bool operator==(const KshUnknownInfo&) const = default;
	};

	struct CompatInfo
	{
		std::string kshVersion;
		KshUnknownInfo kshUnknown;

		[[nodiscard]]
		bool isKshVersionOlderThan(int kshVersionInt) const;
	};
}
