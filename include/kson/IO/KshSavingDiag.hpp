#pragma once
#include "kson/Common/Common.hpp"
#include "IDiag.hpp"
#include "WarningScope.hpp"
#include <string>
#include <variant>
#include <vector>

namespace kson
{
	enum class KshCameraParam
	{
		ZoomTop,
		ZoomBottom,
		ZoomSide,
	};

	struct BpmClampedWarningDetails
	{
		Pulse pulse;
		double value;
		double maxValue;
	};

	struct ZoomValueClampedWarningDetails
	{
		Pulse pulse;
		KshCameraParam param;
		bool isFinalValue;
		double minValue;
		double maxValue;
	};

	struct CenterSplitClampedWarningDetails
	{
		Pulse pulse;
		bool isFinalValue;
		double minValue;
		double maxValue;
	};

	struct ManualTiltClampedWarningDetails
	{
		Pulse pulse;
		bool isFinalValue;
		double minValue;
		double maxValue;
	};

	struct RotationDegClampedWarningDetails
	{
		Pulse pulse;
		bool isFinalValue;
		double minValue;
		double maxValue;
	};

	struct ZoomFractionLostWarningDetails
	{
		std::vector<KshCameraParam> params;
	};

	struct LaserPrecisionLostWarningDetails
	{
	};

	struct FXLongEventParamsLostWarningDetails
	{
		Pulse pulse;
		size_t laneIdx;
		std::string effectName;
	};

	using KshSavingWarningDetails = std::variant<
		BpmClampedWarningDetails,
		ZoomValueClampedWarningDetails,
		CenterSplitClampedWarningDetails,
		ManualTiltClampedWarningDetails,
		RotationDegClampedWarningDetails,
		ZoomFractionLostWarningDetails,
		LaserPrecisionLostWarningDetails,
		FXLongEventParamsLostWarningDetails>;

	struct KshSavingWarning
	{
		WarningScope scope;
		std::string message;
		KshSavingWarningDetails details;
	};

	struct KshSavingDiag : IDiag
	{
		std::vector<KshSavingWarning> warnings;

		std::vector<std::string> playerWarnings() const override;
		std::vector<std::string> editorWarnings() const override;
	};
}
