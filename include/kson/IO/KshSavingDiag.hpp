#pragma once
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	enum class KshSavingWarningType
	{
		BpmClamped,
		ZoomValueClamped,
		CenterSplitClamped,
		ManualTiltClamped,
		RotationDegClamped,
	};

	struct KshSavingWarning
	{
		KshSavingWarningType type;
		WarningScope scope;
		std::string message;
	};

	struct KshSavingDiag : IDiag
	{
		std::vector<KshSavingWarning> warnings;

		std::vector<std::string> toStrings() const override;
	};
}
