#pragma once
#include <string>
#include <vector>

namespace kson
{
	struct IDiag
	{
		virtual ~IDiag() = default;

		// Player warnings
		[[nodiscard]]
		virtual std::vector<std::string> playerWarnings() const = 0;

		// All warnings (including EditorOnly)
		[[nodiscard]]
		virtual std::vector<std::string> editorWarnings() const = 0;
	};
}
