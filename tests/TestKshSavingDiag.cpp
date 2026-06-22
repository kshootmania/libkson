#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <sstream>

extern std::string g_assetsDir;

namespace
{
	kson::ChartData MakeMinimalChartData()
	{
		kson::ChartData chartData;
		chartData.beat.bpm[0] = 120.0;
		chartData.beat.timeSig[0] = kson::TimeSig{ 4, 4 };
		return chartData;
	}

	// Save to KSH and return warnings
	std::vector<kson::KshSavingWarning> SaveAndGetWarnings(const kson::ChartData& chartData)
	{
		kson::KshSavingDiag diag;
		std::ostringstream oss;
		kson::SaveKshChartData(oss, chartData, &diag);
		return diag.warnings;
	}

	template<typename T>
	std::vector<kson::KshSavingWarning> FilterByDetails(const std::vector<kson::KshSavingWarning>& warnings)
	{
		std::vector<kson::KshSavingWarning> result;
		for (const auto& w : warnings)
		{
			if (std::holds_alternative<T>(w.details))
			{
				result.push_back(w);
			}
		}
		return result;
	}
}

TEST_CASE("KSH saving no data loss warnings for Gram[EX]", "[ksh_saving_diag][bundled]")
{
	auto chartData = kson::LoadKshChartData(g_assetsDir + "/Gram_ex.ksh");
	REQUIRE(chartData.error == kson::ErrorType::None);

	auto warnings = SaveAndGetWarnings(chartData);
	auto zoomWarnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(warnings);
	auto laserWarnings = FilterByDetails<kson::LaserPrecisionLostWarningDetails>(warnings);
	auto fxWarnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(warnings);

	REQUIRE(zoomWarnings.empty());
	REQUIRE(laserWarnings.empty());
	REQUIRE(fxWarnings.empty());
}

TEST_CASE("KSH saving ZoomFractionLost warning", "[ksh_saving_diag]")
{
	SECTION("no warning for integer zoom values")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(100.0);

		auto warnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for fractional zoom_top")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(2.5);

		auto warnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_top") != std::string::npos);
		const auto& details = std::get<kson::ZoomFractionLostWarningDetails>(warnings[0].details);
		REQUIRE(details.params == std::vector{ kson::KshCameraParam::ZoomTop });
	}

	SECTION("warning lists multiple fractional zoom params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(2.5);
		chartData.camera.cam.body.zoomSide[0] = kson::GraphPoint(1.3);

		auto warnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_top") != std::string::npos);
		REQUIRE(warnings[0].message.find("zoom_side") != std::string::npos);
		const auto& details = std::get<kson::ZoomFractionLostWarningDetails>(warnings[0].details);
		REQUIRE(details.params == std::vector{ kson::KshCameraParam::ZoomTop, kson::KshCameraParam::ZoomSide });
	}

	SECTION("fractional vf triggers warning")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomBottom[0] = kson::GraphPoint(kson::GraphValue{ 10.0, 3.7 });

		auto warnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_bottom") != std::string::npos);
		const auto& details = std::get<kson::ZoomFractionLostWarningDetails>(warnings[0].details);
		REQUIRE(details.params == std::vector{ kson::KshCameraParam::ZoomBottom });
	}
}

TEST_CASE("KSH saving positional warning details", "[ksh_saving_diag]")
{
	auto chartData = MakeMinimalChartData();
	chartData.meta.dispBPM = "120-200";
	chartData.beat.bpm[kson::kResolution4] = 100000.0;
	chartData.camera.cam.body.zoomTop[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.cam.body.centerSplit[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.tilt[kson::kResolution4] = kson::TiltGraphPoint{ kson::TiltGraphValue{ 100000.0 } };
	chartData.camera.cam.body.rotationDeg[kson::kResolution4] = kson::GraphPoint(100000.0);

	const auto warnings = SaveAndGetWarnings(chartData);
	const auto bpmWarnings = FilterByDetails<kson::BpmClampedWarningDetails>(warnings);
	const auto zoomWarnings = FilterByDetails<kson::ZoomValueClampedWarningDetails>(warnings);
	const auto centerSplitWarnings = FilterByDetails<kson::CenterSplitClampedWarningDetails>(warnings);
	const auto manualTiltWarnings = FilterByDetails<kson::ManualTiltClampedWarningDetails>(warnings);
	const auto rotationWarnings = FilterByDetails<kson::RotationDegClampedWarningDetails>(warnings);

	REQUIRE(bpmWarnings.size() == 1);
	REQUIRE(zoomWarnings.size() == 1);
	REQUIRE(centerSplitWarnings.size() == 1);
	REQUIRE(manualTiltWarnings.size() == 1);
	REQUIRE(rotationWarnings.size() == 1);
	REQUIRE(std::get<kson::BpmClampedWarningDetails>(bpmWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::ZoomValueClampedWarningDetails>(zoomWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::CenterSplitClampedWarningDetails>(centerSplitWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::ManualTiltClampedWarningDetails>(manualTiltWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::RotationDegClampedWarningDetails>(rotationWarnings[0].details).pulse == kson::kResolution4);
}

TEST_CASE("KSH saving LaserPrecisionLost warning", "[ksh_saving_diag]")
{
	SECTION("no warning for grid-aligned laser")
	{
		auto chartData = MakeMinimalChartData();
		kson::LaserSection section;
		section.v[0] = kson::GraphPoint(0.0);
		section.v[kson::kResolution] = kson::GraphPoint(0.5); // 25/50
		chartData.note.laser[0][0] = section;

		auto warnings = FilterByDetails<kson::LaserPrecisionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for off-grid laser")
	{
		auto chartData = MakeMinimalChartData();
		kson::LaserSection section;
		section.v[0] = kson::GraphPoint(0.0);
		section.v[kson::kResolution] = kson::GraphPoint(0.333);
		chartData.note.laser[0][0] = section;

		auto warnings = FilterByDetails<kson::LaserPrecisionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(std::holds_alternative<kson::LaserPrecisionLostWarningDetails>(warnings[0].details));
	}

	SECTION("no warning for empty laser")
	{
		auto chartData = MakeMinimalChartData();

		auto warnings = FilterByDetails<kson::LaserPrecisionLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}
}

TEST_CASE("KSH saving FXLongEventParamsLost warning", "[ksh_saving_diag]")
{
	SECTION("no warning for preservable params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = { { "wave_length", "1/4" } };

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for non-preservable params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = {
			{ "wave_length", "1/4" },
			{ "mix", "50%" },
		};

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("wave_length") == std::string::npos);
		const auto& details = std::get<kson::FXLongEventParamsLostWarningDetails>(warnings[0].details);
		REQUIRE(details.pulse == 0);
		REQUIRE(details.laneIdx == 0);
		REQUIRE(details.effectName == "retrigger");
	}

	SECTION("all params lost for unsupported effect")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["flanger"][0][0] = { { "depth", "50%" } };

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		const auto& details = std::get<kson::FXLongEventParamsLostWarningDetails>(warnings[0].details);
		REQUIRE(details.pulse == 0);
		REQUIRE(details.laneIdx == 0);
		REQUIRE(details.effectName == "flanger");
	}

	SECTION("warnings are separated by note position")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["flanger"][0][0] = { { "depth", "50%" } };
		chartData.audio.audioEffect.fx.longEvent["flanger"][1][kson::kResolution4] = { { "mix", "50%" } };

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 2);
		const auto& firstDetails = std::get<kson::FXLongEventParamsLostWarningDetails>(warnings[0].details);
		const auto& secondDetails = std::get<kson::FXLongEventParamsLostWarningDetails>(warnings[1].details);
		REQUIRE(firstDetails.pulse == 0);
		REQUIRE(firstDetails.laneIdx == 0);
		REQUIRE(secondDetails.pulse == kson::kResolution4);
		REQUIRE(secondDetails.laneIdx == 1);
	}

	SECTION("wave_length in non-fractional format")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = { { "wave_length", "100ms" } };

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
	}

	SECTION("no warning when no long events")
	{
		auto chartData = MakeMinimalChartData();

		auto warnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}
}

TEST_CASE("KSH saving multiple data loss warnings", "[ksh_saving_diag]")
{
	auto chartData = MakeMinimalChartData();

	// Fractional zoom
	chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(2.5);

	// Off-grid laser
	kson::LaserSection section;
	section.v[0] = kson::GraphPoint(0.0);
	section.v[kson::kResolution] = kson::GraphPoint(0.333);
	chartData.note.laser[0][0] = section;

	// Unsupported FX params
	chartData.audio.audioEffect.fx.longEvent["flanger"][0][0] = { { "depth", "50%" } };

	auto warnings = SaveAndGetWarnings(chartData);
	auto zoomWarnings = FilterByDetails<kson::ZoomFractionLostWarningDetails>(warnings);
	auto laserWarnings = FilterByDetails<kson::LaserPrecisionLostWarningDetails>(warnings);
	auto fxWarnings = FilterByDetails<kson::FXLongEventParamsLostWarningDetails>(warnings);

	REQUIRE(zoomWarnings.size() == 1);
	REQUIRE(laserWarnings.size() == 1);
	REQUIRE(fxWarnings.size() == 1);
}
