#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	namespace Encoding
	{
		std::string ShiftJISToUTF8(std::string_view shiftJISStr);
	}
}
