#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct EditorInfo
	{
		std::string appName;

		std::string appVersion;

		ByPulseMulti<std::string> comment;
	};
}
