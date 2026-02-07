#pragma once
#include "WarningScope.hpp"
#include <string>
#include <vector>

namespace kson
{
	enum class KsonWarningType
	{
		InvalidGraphValueFormat,
		InvalidByPulseEntryFormat,
		InvalidGraphEntryFormat,
		InvalidByMeasureIdxEntryFormat,
		InvalidNoteEntryFormat,
		InvalidLaserSectionFormat,
		MissingFormatVersion,
		InvalidFormatVersion,
		JsonParseError,
		JsonTypeError,
		UnexpectedError,
	};

	struct KsonWarning
	{
		KsonWarningType type;
		WarningScope scope;
		std::string message;
	};

	struct KsonParserDiag
	{
		std::vector<KsonWarning> warnings;

		std::vector<std::string> toStrings() const;
	};
}
