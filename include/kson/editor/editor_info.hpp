#pragma once
#include "kson/common/common.hpp"

namespace kson
{
	struct EditorInfo
	{
		std::string appName;

		std::string appVersion;

		ByPulse<std::string> comment;
	};
}
