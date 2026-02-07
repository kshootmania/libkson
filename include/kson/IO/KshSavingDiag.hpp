#pragma once
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	struct KshSavingWarning
	{
		WarningScope scope;
		std::string message;
	};

	struct KshSavingDiag : IDiag
	{
		std::vector<KshSavingWarning> warnings;

		std::vector<std::string> toStrings() const override;
	};
}
