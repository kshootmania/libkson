#pragma once
#include <string>
#include <vector>

namespace kson
{
	struct IDiag
	{
		virtual ~IDiag() = default;

		[[nodiscard]]
		virtual std::vector<std::string> toStrings() const = 0;
	};
}
