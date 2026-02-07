#pragma once
#include <string>
#include <vector>

namespace kson
{
	struct KsonParserDiag
	{
		std::vector<std::string> warnings;
	};
}
