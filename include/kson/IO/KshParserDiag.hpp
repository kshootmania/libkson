#pragma once
#include <string>
#include <vector>

namespace kson
{
	struct KshParserDiag
	{
		std::vector<std::string> warnings;
		bool hasSub32thSlamLasers = false;
	};
}
