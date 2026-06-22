#pragma once
#include "kson/Common/Common.hpp"
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	struct OverlappingLaserSectionsWarningDetails
	{
		Pulse pulse;
		size_t laneIdx;
	};

	struct KsonSavingWarning
	{
		WarningScope scope;
		std::string message;
		OverlappingLaserSectionsWarningDetails details;
	};

	struct KsonSavingDiag : IDiag
	{
		std::vector<KsonSavingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
