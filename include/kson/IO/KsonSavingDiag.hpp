#pragma once
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	enum class KsonSavingWarningType
	{
		OverlappingLaserSections,
	};

	struct KsonSavingWarning
	{
		KsonSavingWarningType type;
		WarningScope scope;
		std::string message;
	};

	struct KsonSavingDiag : IDiag
	{
		std::vector<KsonSavingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
