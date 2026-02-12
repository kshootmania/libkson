#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	namespace Encoding
	{
		[[nodiscard]]
		std::string ShiftJISToUTF8(std::string_view shiftJISStr);
	}
}
