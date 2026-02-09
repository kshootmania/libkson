#pragma once
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	enum class KsonLoadingWarningType
	{
		InvalidGraphValueFormat,
		InvalidByPulseEntryFormat,
		InvalidGraphEntryFormat,
		InvalidByMeasureIdxEntryFormat,
		InvalidNoteEntryFormat,
		InvalidLaserSectionFormat,
		MissingFormatVersion,
		InvalidFormatVersion,
		NewerFormatVersion,
		JsonParseError,
		JsonTypeError,
		UnexpectedError,
	};

	struct KsonLoadingWarning
	{
		KsonLoadingWarningType type;
		WarningScope scope;
		std::string message;
	};

	struct KsonLoadingDiag : IDiag
	{
		std::vector<KsonLoadingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
