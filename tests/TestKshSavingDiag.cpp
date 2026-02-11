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

	// Filter warnings by type
	std::vector<kson::KshSavingWarning> FilterByType(const std::vector<kson::KshSavingWarning>& warnings, kson::KshSavingWarningType type)
	{
		std::vector<kson::KshSavingWarning> result;
		for (const auto& w : warnings)
		{
			if (w.type == type)
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
	auto zoomWarnings = FilterByType(warnings, kson::KshSavingWarningType::ZoomFractionLost);
	auto laserWarnings = FilterByType(warnings, kson::KshSavingWarningType::LaserPrecisionLost);
	auto fxWarnings = FilterByType(warnings, kson::KshSavingWarningType::FXLongEventParamsLost);

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

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::ZoomFractionLost);
		REQUIRE(warnings.empty());
	}

	SECTION("warning for fractional zoom_top")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(2.5);

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::ZoomFractionLost);
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_top") != std::string::npos);
	}

	SECTION("warning lists multiple fractional zoom params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomTop[0] = kson::GraphPoint(2.5);
		chartData.camera.cam.body.zoomSide[0] = kson::GraphPoint(1.3);

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::ZoomFractionLost);
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_top") != std::string::npos);
		REQUIRE(warnings[0].message.find("zoom_side") != std::string::npos);
	}

	SECTION("fractional vf triggers warning")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.body.zoomBottom[0] = kson::GraphPoint(kson::GraphValue{ 10.0, 3.7 });

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::ZoomFractionLost);
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("zoom_bottom") != std::string::npos);
	}
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

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::LaserPrecisionLost);
		REQUIRE(warnings.empty());
	}

	SECTION("warning for off-grid laser")
	{
		auto chartData = MakeMinimalChartData();
		kson::LaserSection section;
		section.v[0] = kson::GraphPoint(0.0);
		section.v[kson::kResolution] = kson::GraphPoint(0.333);
		chartData.note.laser[0][0] = section;

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::LaserPrecisionLost);
		REQUIRE(warnings.size() == 1);
	}

	SECTION("no warning for empty laser")
	{
		auto chartData = MakeMinimalChartData();

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::LaserPrecisionLost);
		REQUIRE(warnings.empty());
	}
}

TEST_CASE("KSH saving FXLongEventParamsLost warning", "[ksh_saving_diag]")
{
	SECTION("no warning for preservable params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = { { "wave_length", "1/4" } };

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::FXLongEventParamsLost);
		REQUIRE(warnings.empty());
	}

	SECTION("warning for non-preservable params")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = {
			{ "wave_length", "1/4" },
			{ "mix", "50%" },
		};

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::FXLongEventParamsLost);
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("\"mix\"") != std::string::npos);
		REQUIRE(warnings[0].message.find("\"wave_length\"") == std::string::npos);
	}

	SECTION("all params lost for unsupported effect")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["flanger"][0][0] = { { "depth", "50%" } };

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::FXLongEventParamsLost);
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].message.find("all parameters will be lost") != std::string::npos);
	}

	SECTION("wave_length in non-fractional format")
	{
		auto chartData = MakeMinimalChartData();
		chartData.audio.audioEffect.fx.longEvent["retrigger"][0][0] = { { "wave_length", "100ms" } };

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::FXLongEventParamsLost);
		REQUIRE(warnings.size() == 1);
	}

	SECTION("no warning when no long events")
	{
		auto chartData = MakeMinimalChartData();

		auto warnings = FilterByType(SaveAndGetWarnings(chartData), kson::KshSavingWarningType::FXLongEventParamsLost);
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
	auto zoomWarnings = FilterByType(warnings, kson::KshSavingWarningType::ZoomFractionLost);
	auto laserWarnings = FilterByType(warnings, kson::KshSavingWarningType::LaserPrecisionLost);
	auto fxWarnings = FilterByType(warnings, kson::KshSavingWarningType::FXLongEventParamsLost);

	REQUIRE(zoomWarnings.size() == 1);
	REQUIRE(laserWarnings.size() == 1);
	REQUIRE(fxWarnings.size() == 1);
}
