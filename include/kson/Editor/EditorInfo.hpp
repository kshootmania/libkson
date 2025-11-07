#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct EditorInfo
	{
		std::string appName;

		std::string appVersion;

		ByPulse<std::string> comment;
	};
}
