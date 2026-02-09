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
		ZoomFractionLost,
		LaserPrecisionLost,
		FXLongEventParamsLost,
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

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
