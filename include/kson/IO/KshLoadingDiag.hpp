#pragma once
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace kson
{
	enum class KshLoadingWarningType
	{
		TitleNotAtBeginning,
		InvalidTimeSig,
		InvalidBPM,
		MissingTimeSigAtZero,
		AudioEffectMissingType,
		AudioEffectInvalidType,
		InvalidAudioEffectParam,
		UncommittedBTNote,
		UncommittedFXNote,
		UndefinedAudioEffect,
		Sub32ndSlamLasers,
		SinglePointLaserSection,
		MeasureSplitNotDivisible,
		AudioEffectDuplicateName,
		UnexpectedError,
	};

	struct KshLoadingWarning
	{
		KshLoadingWarningType type;
		WarningScope scope;
		std::string message;
		std::int64_t lineNo;
	};

	struct KshLoadingDiag : IDiag
	{
		std::vector<KshLoadingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
