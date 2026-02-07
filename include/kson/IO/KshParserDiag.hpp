#pragma once
#include "WarningScope.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace kson
{
	enum class KshWarningType
	{
		TitleNotAtBeginning,
		MissingTimeSigAtZero,
		AudioEffectMissingType,
		AudioEffectInvalidType,
		UncommittedBTNote,
		UncommittedFXNote,
		UndefinedAudioEffect,
		Sub32thSlamLasers,
	};

	struct KshWarning
	{
		KshWarningType type;
		WarningScope scope;
		std::string message;
		std::int64_t lineNo;
	};

	struct KshParserDiag
	{
		std::vector<KshWarning> warnings;

		bool hasSub32thSlamLasers() const;
		std::vector<std::string> toStrings() const;
	};
}
