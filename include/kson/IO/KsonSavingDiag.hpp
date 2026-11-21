#pragma once
#include "kson/Common/Common.hpp"
#include "IDiag.hpp"
#include "SavingWarningDetails.hpp"
#include "WarningScope.hpp"
#include <string>
#include <variant>
#include <vector>

namespace kson
{
	struct OverlappingLaserSectionsWarningDetails
	{
		Pulse pulse;
		size_t laneIdx;
	};

	using KsonSavingWarningDetails = std::variant<
		OverlappingLaserSectionsWarningDetails,
		CamPatternUndefinedWarningDetails>;

	struct KsonSavingWarning
	{
		WarningScope scope;
		std::string message;
		KsonSavingWarningDetails details;
	};

	struct KsonSavingDiag : IDiag
	{
		std::vector<KsonSavingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
