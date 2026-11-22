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
	chartData.camera.cam.body.highwayWipe[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.tilt[kson::kResolution4] = kson::TiltGraphPoint{ kson::TiltGraphValue{ 100000.0 } };
	chartData.camera.cam.body.rotationDeg[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.cam.body.rotationDegHighway[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.cam.body.rotationDegJdgline[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.cam.body.rotationDegLayer[kson::kResolution4] = kson::GraphPoint(100000.0);
	chartData.camera.cam.pattern.laser.slamEvent.spin[kson::kResolution4] = kson::CamPatternInvokeSpin{ .d = 1, .length = kson::kResolution4, .v = { .count = 65536 } };
	chartData.camera.cam.pattern.laser.slamEvent.halfSpin[kson::kResolution4 * 2] = kson::CamPatternInvokeHalfSpin{ .d = 1, .length = kson::kResolution4, .v = { .deg = 100000.0 } };

	const auto warnings = SaveAndGetWarnings(chartData);
	const auto bpmWarnings = FilterByDetails<kson::BpmClampedWarningDetails>(warnings);
	const auto zoomWarnings = FilterByDetails<kson::ZoomValueClampedWarningDetails>(warnings);
	const auto centerSplitWarnings = FilterByDetails<kson::CenterSplitClampedWarningDetails>(warnings);
	const auto highwayWipeWarnings = FilterByDetails<kson::HighwayWipeClampedWarningDetails>(warnings);
	const auto manualTiltWarnings = FilterByDetails<kson::ManualTiltClampedWarningDetails>(warnings);
	const auto rotationWarnings = FilterByDetails<kson::RotationDegClampedWarningDetails>(warnings);
	const auto rotationHighwayWarnings = FilterByDetails<kson::RotationDegHighwayClampedWarningDetails>(warnings);
	const auto rotationJdglineWarnings = FilterByDetails<kson::RotationDegJdglineClampedWarningDetails>(warnings);
	const auto rotationLayerWarnings = FilterByDetails<kson::RotationDegLayerClampedWarningDetails>(warnings);
	const auto spinCountWarnings = FilterByDetails<kson::SpinCountClampedWarningDetails>(warnings);
	const auto halfSpinDegWarnings = FilterByDetails<kson::HalfSpinDegClampedWarningDetails>(warnings);

	REQUIRE(bpmWarnings.size() == 1);
	REQUIRE(zoomWarnings.size() == 1);
	REQUIRE(centerSplitWarnings.size() == 1);
	REQUIRE(highwayWipeWarnings.size() == 1);
	REQUIRE(manualTiltWarnings.size() == 1);
	REQUIRE(rotationWarnings.size() == 1);
	REQUIRE(rotationHighwayWarnings.size() == 1);
	REQUIRE(rotationJdglineWarnings.size() == 1);
	REQUIRE(rotationLayerWarnings.size() == 1);
	REQUIRE(spinCountWarnings.size() == 1);
	REQUIRE(halfSpinDegWarnings.size() == 1);
	REQUIRE(std::get<kson::BpmClampedWarningDetails>(bpmWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::ZoomValueClampedWarningDetails>(zoomWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::CenterSplitClampedWarningDetails>(centerSplitWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::HighwayWipeClampedWarningDetails>(highwayWipeWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::ManualTiltClampedWarningDetails>(manualTiltWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::RotationDegClampedWarningDetails>(rotationWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::RotationDegHighwayClampedWarningDetails>(rotationHighwayWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::RotationDegJdglineClampedWarningDetails>(rotationJdglineWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::RotationDegLayerClampedWarningDetails>(rotationLayerWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::SpinCountClampedWarningDetails>(spinCountWarnings[0].details).pulse == kson::kResolution4);
	REQUIRE(std::get<kson::HalfSpinDegClampedWarningDetails>(halfSpinDegWarnings[0].details).pulse == kson::kResolution4 * 2);
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

TEST_CASE("KSH saving CamPatternPrecisionLost warning", "[ksh_saving_diag]")
{
	SECTION("warning for off-grid position")
	{
		auto chartData = MakeMinimalChartData();
		kson::CamPatternDef def;
		def.body.rotationDeg.emplace(0, kson::GraphPoint{ 0.0 });
		def.body.rotationDeg.emplace(7, kson::GraphPoint{ 10.0 });
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = def });

		const auto warnings = SaveAndGetWarnings(chartData);
		const auto filtered = FilterByDetails<kson::CamPatternPrecisionLostWarningDetails>(warnings);
		REQUIRE(filtered.size() == 1);
		REQUIRE(std::get<kson::CamPatternPrecisionLostWarningDetails>(filtered[0].details).name == "Pattern");
	}

	SECTION("warning for fractional zoom value")
	{
		auto chartData = MakeMinimalChartData();
		kson::CamPatternDef def;
		def.body.zoomTop.emplace(0, kson::GraphPoint{ 0.5 });
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = def });

		const auto warnings = SaveAndGetWarnings(chartData);
		REQUIRE(FilterByDetails<kson::CamPatternPrecisionLostWarningDetails>(warnings).size() == 1);
	}

	SECTION("no warning for grid-aligned integer values")
	{
		auto chartData = MakeMinimalChartData();
		kson::CamPatternDef def;
		def.body.zoomTop.emplace(0, kson::GraphPoint{ 0.0 });
		def.body.zoomTop.emplace(kson::kResolution, kson::GraphPoint{ 100.0 });
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = def });

		const auto warnings = SaveAndGetWarnings(chartData);
		REQUIRE(FilterByDetails<kson::CamPatternPrecisionLostWarningDetails>(warnings).empty());
	}
}

TEST_CASE("KSH saving CamPatternInvalidName warning", "[ksh_saving_diag]")
{
	SECTION("no warning for valid name")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = {} });

		auto warnings = FilterByDetails<kson::CamPatternInvalidNameWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for name with separator")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.pattern.def.push_back({ .name = "Two Words", .v = {} });
		chartData.camera.cam.pattern.laser.slamEvent.custom["Two Words"].emplace(0, kson::CamPatternInvokeCustom{ .d = 1, .length = kson::kResolution4 });

		std::ostringstream oss;
		kson::KshSavingDiag kshSavingDiag;
		REQUIRE(kson::SaveKshChartData(oss, chartData, &kshSavingDiag) == kson::ErrorType::None);

		auto warnings = FilterByDetails<kson::CamPatternInvalidNameWarningDetails>(kshSavingDiag.warnings);
		REQUIRE(warnings.size() == 1);
		REQUIRE(std::get<kson::CamPatternInvalidNameWarningDetails>(warnings[0].details).name == "Two Words");
		REQUIRE(oss.str().find("#define_cam") == std::string::npos);
		REQUIRE(oss.str().find("cam_slam_") == std::string::npos);
	}
}

TEST_CASE("KSH saving CamPatternUndefined warning", "[ksh_saving_diag]")
{
	SECTION("no warning for defined name")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = {} });
		chartData.camera.cam.pattern.laser.slamEvent.custom["Pattern"].emplace(0, kson::CamPatternInvokeCustom{ .d = 1, .length = kson::kResolution4 });

		auto warnings = FilterByDetails<kson::CamPatternUndefinedWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for undefined name")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = {} });
		chartData.camera.cam.pattern.laser.slamEvent.custom["Other"].emplace(0, kson::CamPatternInvokeCustom{ .d = 1, .length = kson::kResolution4 });
		chartData.camera.cam.pattern.laser.slamEvent.custom["Other"].emplace(960, kson::CamPatternInvokeCustom{ .d = -1, .length = kson::kResolution4 });

		auto warnings = FilterByDetails<kson::CamPatternUndefinedWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(std::get<kson::CamPatternUndefinedWarningDetails>(warnings[0].details).name == "Other");
		REQUIRE(warnings[0].scope == kson::WarningScope::EditorOnly);
	}

	SECTION("warning for undefined name that cannot be used in KSH")
	{
		auto chartData = MakeMinimalChartData();
		chartData.camera.cam.pattern.laser.slamEvent.custom["A B"].emplace(0, kson::CamPatternInvokeCustom{ .d = 1, .length = kson::kResolution4 });

		auto warnings = FilterByDetails<kson::CamPatternUndefinedWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(std::get<kson::CamPatternUndefinedWarningDetails>(warnings[0].details).name == "A B");
	}
}

TEST_CASE("KSH saving CamPatternValueClamped warning", "[ksh_saving_diag]")
{
	SECTION("no warning for value in range")
	{
		auto chartData = MakeMinimalChartData();
		kson::CamPatternDef def;
		def.body.zoomTop.emplace(0, kson::GraphPoint{ 65535.0 });
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = def });

		auto warnings = FilterByDetails<kson::CamPatternValueClampedWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for value out of range")
	{
		auto chartData = MakeMinimalChartData();
		kson::CamPatternDef def;
		def.body.zoomTop.emplace(0, kson::GraphPoint{ 1e20 });
		def.body.rotationDeg.emplace(0, kson::GraphPoint{ -100000.0 });
		chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = def });

		std::ostringstream oss;
		kson::KshSavingDiag kshSavingDiag;
		REQUIRE(kson::SaveKshChartData(oss, chartData, &kshSavingDiag) == kson::ErrorType::None);

		auto warnings = FilterByDetails<kson::CamPatternValueClampedWarningDetails>(kshSavingDiag.warnings);
		REQUIRE(warnings.size() == 1);
		REQUIRE(std::get<kson::CamPatternValueClampedWarningDetails>(warnings[0].details).name == "Pattern");
		REQUIRE(oss.str().find("#define_cam Pattern zoom_top=0:65535;rotation_deg=0:-65535\r\n") != std::string::npos);
	}
}

TEST_CASE("KSH saving CamPatternScaleClamped warning", "[ksh_saving_diag]")
{
	auto chartData = MakeMinimalChartData();
	chartData.camera.cam.pattern.def.push_back({ .name = "Pattern", .v = {} });
	chartData.camera.cam.pattern.laser.slamEvent.custom["Pattern"].emplace(0, kson::CamPatternInvokeCustom{ .d = 1, .length = kson::kResolution4, .v = { .scale = 100000.0 } });

	std::ostringstream oss;
	kson::KshSavingDiag kshSavingDiag;
	REQUIRE(kson::SaveKshChartData(oss, chartData, &kshSavingDiag) == kson::ErrorType::None);

	auto warnings = FilterByDetails<kson::CamPatternScaleClampedWarningDetails>(kshSavingDiag.warnings);
	REQUIRE(warnings.size() == 1);
	REQUIRE(std::get<kson::CamPatternScaleClampedWarningDetails>(warnings[0].details).name == "Pattern");
	REQUIRE(oss.str().find("cam_slam_r=Pattern;192;65535\r\n") != std::string::npos);
}

TEST_CASE("KSH saving MovieValueClamped warning", "[ksh_saving_diag]")
{
	auto chartData = MakeMinimalChartData();
	chartData.bg.legacy.movie.filename = "movie.mp4";
	chartData.bg.legacy.movie.opacity = 1e300;
	chartData.bg.legacy.movie.scale = -1e300;
	chartData.bg.legacy.movie.x = 1e300;
	chartData.bg.legacy.movie.y = -1e300;

	std::ostringstream oss;
	kson::KshSavingDiag kshSavingDiag;
	REQUIRE(kson::SaveKshChartData(oss, chartData, &kshSavingDiag) == kson::ErrorType::None);

	auto warnings = FilterByDetails<kson::MovieValueClampedWarningDetails>(kshSavingDiag.warnings);
	REQUIRE(warnings.size() == 4);
	REQUIRE(oss.str().find("v_opacity=") == std::string::npos);
	REQUIRE(oss.str().find("v_scale=0\r\n") != std::string::npos);
	REQUIRE(oss.str().find("vx=10000\r\n") != std::string::npos);
	REQUIRE(oss.str().find("vy=-10000\r\n") != std::string::npos);
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

TEST_CASE("KSH saving AdjacentLaserSections warning", "[ksh_saving_diag]")
{
	SECTION("no warning for separated sections")
	{
		auto chartData = MakeMinimalChartData();
		kson::LaserSection section1;
		section1.v[0] = kson::GraphPoint(0.0);
		section1.v[kson::kResolution] = kson::GraphPoint(1.0);
		chartData.note.laser[0][0] = section1;
		kson::LaserSection section2;
		section2.v[0] = kson::GraphPoint(0.5);
		section2.v[kson::kResolution] = kson::GraphPoint(1.0);
		chartData.note.laser[0][kson::kResolution * 2] = section2;

		auto warnings = FilterByDetails<kson::AdjacentLaserSectionsWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.empty());
	}

	SECTION("warning for a section starting at the end of the previous one")
	{
		auto chartData = MakeMinimalChartData();
		kson::LaserSection section1;
		section1.v[0] = kson::GraphPoint(0.0);
		section1.v[kson::kResolution] = kson::GraphPoint(1.0);
		chartData.note.laser[1][0] = section1;
		kson::LaserSection section2;
		section2.v[0] = kson::GraphPoint(0.5);
		section2.v[kson::kResolution] = kson::GraphPoint(1.0);
		chartData.note.laser[1][kson::kResolution] = section2;

		auto warnings = FilterByDetails<kson::AdjacentLaserSectionsWarningDetails>(SaveAndGetWarnings(chartData));
		REQUIRE(warnings.size() == 1);
		REQUIRE(warnings[0].scope == kson::WarningScope::EditorOnly);
		REQUIRE(warnings[0].message.find("LASER-R") != std::string::npos);
		const auto& details = std::get<kson::AdjacentLaserSectionsWarningDetails>(warnings[0].details);
		REQUIRE(details.pulse == kson::kResolution);
		REQUIRE(details.laneIdx == 1);
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
