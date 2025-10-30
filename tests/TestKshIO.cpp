#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/io/ksh_io.hpp>
#include <kson/io/kson_io.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

extern std::string g_assetsDir;
extern std::filesystem::path g_exeDir;

TEST_CASE("KSON I/O round-trip (bundled charts)", "[ksh_io][kson_io][round_trip]") {
    auto testRoundTrip = [](const std::string& filename) {
        auto chart1 = kson::LoadKSHChartData(filename);

        if (chart1.error != kson::ErrorType::None) {
            std::cerr << "Error loading KSH file: " << filename << std::endl;
            std::cerr << "Error code: " << static_cast<int>(chart1.error) << std::endl;
            std::cerr << "Warnings count: " << chart1.warnings.size() << std::endl;
            for (const auto& warning : chart1.warnings) {
                std::cerr << "  - " << warning << std::endl;
            }
        }
        REQUIRE(chart1.error == kson::ErrorType::None);
        
        std::ostringstream oss1;
        auto saveResult1 = kson::SaveKSONChartData(oss1, chart1);
        REQUIRE(saveResult1 == kson::ErrorType::None);
        std::string ksonString1 = oss1.str();
        REQUIRE(!ksonString1.empty());
        
        std::istringstream iss1(ksonString1);
        auto chart2 = kson::LoadKSONChartData(iss1);
        REQUIRE(chart2.error == kson::ErrorType::None);
        
        std::ostringstream oss2;
        auto saveResult2 = kson::SaveKSONChartData(oss2, chart2);
        REQUIRE(saveResult2 == kson::ErrorType::None);
        std::string ksonString2 = oss2.str();
        REQUIRE(!ksonString2.empty());
        
        nlohmann::json json1 = nlohmann::json::parse(ksonString1);
        nlohmann::json json2 = nlohmann::json::parse(ksonString2);
        
        REQUIRE(json1["format_version"] == json2["format_version"]);
        REQUIRE(json1["meta"] == json2["meta"]);
        REQUIRE(json1["beat"] == json2["beat"]);
        REQUIRE(json1["gauge"] == json2["gauge"]);
        REQUIRE(json1["note"] == json2["note"]);
        REQUIRE(json1["audio"] == json2["audio"]);
        REQUIRE(json1["camera"] == json2["camera"]);
        REQUIRE(json1["bg"] == json2["bg"]);
        
        if (json1.contains("editor") && json2.contains("editor")) {
            REQUIRE(json1["editor"] == json2["editor"]);
        }
        if (json1.contains("compat") && json2.contains("compat")) {
            REQUIRE(json1["compat"] == json2["compat"]);
        }
        if (json1.contains("impl") && json2.contains("impl")) {
            REQUIRE(json1["impl"] == json2["impl"]);
        }
    };
    
    SECTION("Gram[LT]") {
        testRoundTrip(g_assetsDir + "/Gram_lt.ksh");
    }

    SECTION("Gram[CH]") {
        testRoundTrip(g_assetsDir + "/Gram_ch.ksh");
    }

    SECTION("Gram[EX]") {
        testRoundTrip(g_assetsDir + "/Gram_ex.ksh");
    }

    SECTION("Gram[IN]") {
        testRoundTrip(g_assetsDir + "/Gram_in.ksh");
    }
}

TEST_CASE("Gram[EX] detailed chart validation", "[ksh_io][gram]") {
	auto chart = kson::LoadKSONChartData(g_assetsDir + "/Gram_ex.kson");
	REQUIRE(chart.error == kson::ErrorType::None);

	SECTION("Metadata") {
		REQUIRE(chart.meta.title == "Gram");
		REQUIRE(chart.meta.artist == "Blast Smith");
		REQUIRE(chart.meta.chartAuthor == "坂本龍馬");
		REQUIRE(chart.meta.level == 16);
		REQUIRE(chart.meta.difficulty.idx == 2);
		REQUIRE(chart.meta.dispBPM == "191");
		REQUIRE(chart.meta.information == "Schatz des Urtext");
		REQUIRE(chart.meta.jacketAuthor == "Blast Smith");
		REQUIRE(chart.meta.jacketFilename == "gram.jpg");
	}

	SECTION("BT note counts") {
		auto countNotes = [](const kson::ByPulse<kson::Interval>& notes) -> std::pair<int, int> {
			int chipNotes = 0, longNotes = 0;
			for (const auto& [y, note] : notes) {
				if (note.length == 0) {
					++chipNotes;
				} else {
					++longNotes;
				}
			}
			return {chipNotes, longNotes};
		};

		auto [bt0_chipNotes, bt0_longNotes] = countNotes(chart.note.bt[0]);
		REQUIRE(bt0_chipNotes == 104);
		REQUIRE(bt0_longNotes == 5);

		auto [bt1_chipNotes, bt1_longNotes] = countNotes(chart.note.bt[1]);
		REQUIRE(bt1_chipNotes == 199);
		REQUIRE(bt1_longNotes == 3);

		auto [bt2_chipNotes, bt2_longNotes] = countNotes(chart.note.bt[2]);
		REQUIRE(bt2_chipNotes == 176);
		REQUIRE(bt2_longNotes == 3);

		auto [bt3_chipNotes, bt3_longNotes] = countNotes(chart.note.bt[3]);
		REQUIRE(bt3_chipNotes == 111);
		REQUIRE(bt3_longNotes == 6);
	}

	SECTION("FX note counts") {
		auto countNotes = [](const kson::ByPulse<kson::Interval>& notes) -> std::pair<int, int> {
			int chipNotes = 0, longNotes = 0;
			for (const auto& [y, note] : notes) {
				if (note.length == 0) {
					++chipNotes;
				} else {
					++longNotes;
				}
			}
			return {chipNotes, longNotes};
		};

		auto [fx0_chipNotes, fx0_longNotes] = countNotes(chart.note.fx[0]);
		REQUIRE(fx0_chipNotes == 77);
		REQUIRE(fx0_longNotes == 49);

		auto [fx1_chipNotes, fx1_longNotes] = countNotes(chart.note.fx[1]);
		REQUIRE(fx1_chipNotes == 65);
		REQUIRE(fx1_longNotes == 43);
	}

	SECTION("Laser section counts") {
		REQUIRE(chart.note.laser[0].size() == 45);
		REQUIRE(chart.note.laser[1].size() == 44);

		auto countWide = [](const kson::ByPulse<kson::LaserSection>& laser) -> int {
			int count = 0;
			for (const auto& [y, section] : laser) {
				if (section.wide()) {
					++count;
				}
			}
			return count;
		};

		REQUIRE(countWide(chart.note.laser[0]) == 14);
		REQUIRE(countWide(chart.note.laser[1]) == 15);
	}

	SECTION("Camera data") {
		REQUIRE(chart.camera.cam.body.zoomBottom.size() == 72);
		REQUIRE(chart.camera.cam.body.zoomTop.size() == 45);
		REQUIRE(chart.camera.tilt.keep.size() == 8);
		REQUIRE(chart.camera.tilt.scale.size() == 10);
	}

	SECTION("Audio effects") {
		REQUIRE(chart.audio.audioEffect.fx.def.size() == 7);
		REQUIRE(chart.audio.audioEffect.fx.longEvent.size() == 13);
		REQUIRE(chart.audio.audioEffect.laser.def.size() == 2);
		REQUIRE(chart.audio.audioEffect.laser.peakingFilterDelay == 40);
	}

	SECTION("Beat data") {
		REQUIRE(chart.beat.bpm.size() == 1);
		REQUIRE(chart.beat.bpm.contains(0));
		REQUIRE(chart.beat.bpm.at(0) == Approx(191.0));

		REQUIRE(chart.beat.timeSig.size() == 1);
		REQUIRE(chart.beat.timeSig.contains(0));
		REQUIRE(chart.beat.timeSig.at(0).n == 4);
		REQUIRE(chart.beat.timeSig.at(0).d == 4);

		REQUIRE(chart.beat.scrollSpeed.size() == 1);
		REQUIRE(chart.beat.scrollSpeed.contains(0));
		REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
	}

	SECTION("Audio BGM") {
		REQUIRE(chart.audio.bgm.filename == "Gram.ogg");
		REQUIRE(chart.audio.bgm.preview.offset == 59050);
		REQUIRE(chart.audio.bgm.preview.duration == 12573);
	}

	SECTION("Audio key sound") {
		REQUIRE(chart.audio.keySound.laser.vol.size() == 6);
		REQUIRE(chart.audio.keySound.laser.slamEvent.contains("down"));
		REQUIRE(chart.audio.keySound.laser.slamEvent.at("down").size() == 1);
	}

	SECTION("Background") {
		REQUIRE(chart.bg.legacy.bg.size() == 2); // 1 element in KSON, 2 in source code
		REQUIRE(chart.bg.legacy.bg[0].filename == "cyber");
		REQUIRE(chart.bg.legacy.bg[1].filename == "");
		REQUIRE(chart.bg.legacy.layer.filename == "techno");
	}

	SECTION("Camera pattern") {
		REQUIRE(chart.camera.cam.pattern.laser.slamEvent.spin.size() == 1);
		REQUIRE(chart.camera.cam.pattern.laser.slamEvent.swing.size() == 1);
	}

	SECTION("Compat info") {
		REQUIRE(chart.compat.kshVersion == "160");
	}
}

TEST_CASE("KSON I/O round-trip (all songs)", "[.][ksh_io][kson_io][round_trip][all_songs]") {
    auto testRoundTrip = [](const std::string& filename) {
        auto chart1 = kson::LoadKSHChartData(filename);

        if (chart1.error != kson::ErrorType::None) {
            std::cerr << "Error loading KSH file: " << filename << std::endl;
            std::cerr << "Error code: " << static_cast<int>(chart1.error) << std::endl;
            std::cerr << "Warnings count: " << chart1.warnings.size() << std::endl;
            for (const auto& warning : chart1.warnings) {
                std::cerr << "  - " << warning << std::endl;
            }
        }
        REQUIRE(chart1.error == kson::ErrorType::None);
        
        std::ostringstream oss1;
        auto saveResult1 = kson::SaveKSONChartData(oss1, chart1);
        REQUIRE(saveResult1 == kson::ErrorType::None);
        std::string ksonString1 = oss1.str();
        REQUIRE(!ksonString1.empty());
        
        std::istringstream iss1(ksonString1);
        auto chart2 = kson::LoadKSONChartData(iss1);
        REQUIRE(chart2.error == kson::ErrorType::None);
        
        std::ostringstream oss2;
        auto saveResult2 = kson::SaveKSONChartData(oss2, chart2);
        REQUIRE(saveResult2 == kson::ErrorType::None);
        std::string ksonString2 = oss2.str();
        REQUIRE(!ksonString2.empty());
        
        nlohmann::json json1 = nlohmann::json::parse(ksonString1);
        nlohmann::json json2 = nlohmann::json::parse(ksonString2);
        
        REQUIRE(json1["format_version"] == json2["format_version"]);
        REQUIRE(json1["meta"] == json2["meta"]);
        REQUIRE(json1["beat"] == json2["beat"]);
        REQUIRE(json1["gauge"] == json2["gauge"]);
        REQUIRE(json1["note"] == json2["note"]);
        REQUIRE(json1["audio"] == json2["audio"]);
        REQUIRE(json1["camera"] == json2["camera"]);
        REQUIRE(json1["bg"] == json2["bg"]);
        
        if (json1.contains("editor") && json2.contains("editor")) {
            REQUIRE(json1["editor"] == json2["editor"]);
        }
        if (json1.contains("compat") && json2.contains("compat")) {
            REQUIRE(json1["compat"] == json2["compat"]);
        }
        if (json1.contains("impl") && json2.contains("impl")) {
            REQUIRE(json1["impl"] == json2["impl"]);
        }
    };
    
    std::filesystem::path songsPath = g_exeDir / "../../../kshootmania/App/songs";
    songsPath = std::filesystem::weakly_canonical(songsPath);
    if (!std::filesystem::exists(songsPath) || !std::filesystem::is_directory(songsPath)) {
        WARN("songs directory not found, skipping all songs test");
        return;
    }

    SECTION("All KSH files in songs directory") {
        std::vector<std::string> kshFiles;
        std::string searchCmd = "find \"" + songsPath.string() + "\" -name \"*.ksh\" 2>/dev/null";
        FILE* pipe = popen(searchCmd.c_str(), "r");
        if (pipe) {
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string filename = buffer;
                if (!filename.empty() && filename.back() == '\n') {
                    filename.pop_back();
                }
                if (!filename.empty()) {
                    kshFiles.push_back(filename);
                }
            }
            pclose(pipe);
        }
        
        if (kshFiles.empty()) {
            WARN("No KSH files found in songs directory");
            return;
        }
        
        for (const auto& file : kshFiles) {
            INFO("Testing file: " << file);
            testRoundTrip(file);
        }
    }
}

TEST_CASE("pfiltergain to Q value conversion", "[ksh_io][pfiltergain]") {
    // Test that pfiltergain values (0, 50, 100) are properly converted to Q values
    const std::string kshContent = R"(title=pfiltergain test
ver=167
--
pfiltergain=0
0000|00|--
0000|00|--
0000|00|--
0000|00|--
--
pfiltergain=50
0000|00|--
0000|00|--
0000|00|--
0000|00|--
--
pfiltergain=100
0000|00|--
0000|00|--
0000|00|--
0000|00|--
--
)";

    std::istringstream stream(kshContent);
    auto chartData = kson::LoadKSHChartData(stream);

    REQUIRE(chartData.error == kson::ErrorType::None);

    // Check peaking_filter gain parameter
    REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("peaking_filter"));
    const auto& peakingParams = chartData.audio.audioEffect.laser.paramChange.at("peaking_filter");
    REQUIRE(peakingParams.contains("gain"));
    const auto& peakingGain = peakingParams.at("gain");

    // Check high_pass_filter q parameter
    REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("high_pass_filter"));
    const auto& hpfParams = chartData.audio.audioEffect.laser.paramChange.at("high_pass_filter");
    REQUIRE(hpfParams.contains("q"));
    const auto& hpfQ = hpfParams.at("q");

    // Check low_pass_filter q parameter
    REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("low_pass_filter"));
    const auto& lpfParams = chartData.audio.audioEffect.laser.paramChange.at("low_pass_filter");
    REQUIRE(lpfParams.contains("q"));
    const auto& lpfQ = lpfParams.at("q");

    // KSH: 1 measure = 192 * 4 = 768 pulse (4/4 time) -> KSON: 240 * 4 = 960 pulse
    constexpr kson::Pulse kMeasurePulse = kson::kResolution4;  // 960

    // pfiltergain=0 (pulse=0) should convert to q=0.7
    REQUIRE(peakingGain.at(0) == "0%");
    REQUIRE(std::abs(std::stod(hpfQ.at(0)) - 0.7) < 0.001);
    REQUIRE(std::abs(std::stod(lpfQ.at(0)) - 0.7) < 0.001);

    // pfiltergain=50 (pulse=960, measure 2) should convert to q=5.0
    REQUIRE(peakingGain.at(kMeasurePulse) == "50%");
    REQUIRE(std::abs(std::stod(hpfQ.at(kMeasurePulse)) - 5.0) < 0.001);
    REQUIRE(std::abs(std::stod(lpfQ.at(kMeasurePulse)) - 5.0) < 0.001);

    // pfiltergain=100 (pulse=1920, measure 3) should convert to q=9.3
    REQUIRE(peakingGain.at(kMeasurePulse * 2) == "100%");
    REQUIRE(std::abs(std::stod(hpfQ.at(kMeasurePulse * 2)) - 9.3) < 0.001);
    REQUIRE(std::abs(std::stod(lpfQ.at(kMeasurePulse * 2)) - 9.3) < 0.001);
}

TEST_CASE("KSH Curve Parameter Loading", "[ksh_io][curve]") {
    constexpr kson::Pulse kMeasurePulse = kson::kResolution4;

    SECTION("Curve before parameter (all parameters in one chart)") {
        std::stringstream ss;
        ss << "title=Curve Test\n";
        ss << "artist=Test\n";
        ss << "effect=Test\n";
        ss << "jacket=\n";
        ss << "illustrator=\n";
        ss << "difficulty=challenge\n";
        ss << "level=1\n";
        ss << "t=120\n";
        ss << "ver=170\n";
        ss << "--\n";
        // Measure 0: curve before parameter
        ss << "zoom_top_curve=0.3;0.7\n";
        ss << "zoom_top=100\n";
        ss << "0000|00|--\n";
        ss << "zoom_bottom_curve=0.4;0.6\n";
        ss << "zoom_bottom=50\n";
        ss << "0000|00|--\n";
        ss << "zoom_side_curve=0.5;0.5\n";
        ss << "zoom_side=-25\n";
        ss << "0000|00|--\n";
        ss << "center_split_curve=0.2;0.8\n";
        ss << "center_split=200\n";
        ss << "scroll_speed_curve=0.3;0.6\n";
        ss << "scroll_speed=1.5\n";
        ss << "0000|00|--\n";
        ss << "tilt_curve=0.1;0.9\n";
        ss << "tilt=0.1\n";
        ss << "0000|00|--\n";
        ss << "laser_l_curve=0.6;0.4\n";
        ss << "laser_r_curve=0.7;0.3\n";
        ss << "0000|00|0o\n";
        ss << "0000|00|::\n";
        ss << "0000|00|o0\n";
        ss << "--\n";

        kson::ChartData chart = kson::LoadKSHChartData(ss);
        REQUIRE(chart.error == kson::ErrorType::None);

        REQUIRE(chart.camera.cam.body.zoomTop.contains(0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.a == Approx(0.3));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.b == Approx(0.7));

        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 8));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.a == Approx(0.4));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.b == Approx(0.6));

        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.a == Approx(0.5));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.b == Approx(0.5));

        REQUIRE(chart.camera.cam.body.centerSplit.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).v.v == Approx(200.0));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.2));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.8));

        REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.v == Approx(1.5));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.3));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.6));

        REQUIRE(chart.camera.tilt.manual.contains(kMeasurePulse / 2));
        const auto& tiltSection = chart.camera.tilt.manual.at(kMeasurePulse / 2);
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.9));

        REQUIRE(chart.note.laser[0].contains(kMeasurePulse * 5 / 8));
        const auto& laserL = chart.note.laser[0].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserL.v.contains(0));
        REQUIRE(laserL.v.at(0).v.v == Approx(0.0));
        REQUIRE(laserL.v.at(0).curve.a == Approx(0.6));
        REQUIRE(laserL.v.at(0).curve.b == Approx(0.4));
        REQUIRE(laserL.v.contains(240));
        REQUIRE(laserL.v.at(240).v.v == Approx(1.0));

        REQUIRE(chart.note.laser[1].contains(kMeasurePulse * 5 / 8));
        const auto& laserR = chart.note.laser[1].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserR.v.contains(0));
        REQUIRE(laserR.v.at(0).v.v == Approx(1.0));
        REQUIRE(laserR.v.at(0).curve.a == Approx(0.7));
        REQUIRE(laserR.v.at(0).curve.b == Approx(0.3));
        REQUIRE(laserR.v.contains(240));
        REQUIRE(laserR.v.at(240).v.v == Approx(0.0));
    }

    SECTION("Curve after parameter (all parameters in one chart)") {
        std::stringstream ss;
        ss << "title=Curve Test\n";
        ss << "artist=Test\n";
        ss << "effect=Test\n";
        ss << "jacket=\n";
        ss << "illustrator=\n";
        ss << "difficulty=challenge\n";
        ss << "level=1\n";
        ss << "t=120\n";
        ss << "ver=170\n";
        ss << "--\n";
        // Measure 0: parameter before curve
        ss << "zoom_top=100\n";
        ss << "zoom_top_curve=0.3;0.7\n";
        ss << "0000|00|--\n";
        ss << "zoom_bottom=50\n";
        ss << "zoom_bottom_curve=0.4;0.6\n";
        ss << "0000|00|--\n";
        ss << "zoom_side=-25\n";
        ss << "zoom_side_curve=0.5;0.5\n";
        ss << "0000|00|--\n";
        ss << "center_split=200\n";
        ss << "center_split_curve=0.2;0.8\n";
        ss << "scroll_speed=1.5\n";
        ss << "scroll_speed_curve=0.3;0.6\n";
        ss << "0000|00|--\n";
        ss << "tilt=0.1\n";
        ss << "tilt_curve=0.1;0.9\n";
        ss << "0000|00|--\n";
        ss << "laser_l_curve=0.6;0.4\n";
        ss << "laser_r_curve=0.7;0.3\n";
        ss << "0000|00|0o\n";
        ss << "0000|00|::\n";
        ss << "0000|00|o0\n";
        ss << "--\n";

        kson::ChartData chart = kson::LoadKSHChartData(ss);
        REQUIRE(chart.error == kson::ErrorType::None);

        REQUIRE(chart.camera.cam.body.zoomTop.contains(0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.a == Approx(0.3));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.b == Approx(0.7));

        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 8));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.a == Approx(0.4));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.b == Approx(0.6));

        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.a == Approx(0.5));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.b == Approx(0.5));

        REQUIRE(chart.camera.cam.body.centerSplit.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).v.v == Approx(200.0));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.2));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.8));

        REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.v == Approx(1.5));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.3));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.6));

        REQUIRE(chart.camera.tilt.manual.contains(kMeasurePulse / 2));
        const auto& tiltSection = chart.camera.tilt.manual.at(kMeasurePulse / 2);
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.9));

        REQUIRE(chart.note.laser[0].contains(kMeasurePulse * 5 / 8));
        const auto& laserL = chart.note.laser[0].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserL.v.contains(0));
        REQUIRE(laserL.v.at(0).v.v == Approx(0.0));
        REQUIRE(laserL.v.at(0).curve.a == Approx(0.6));
        REQUIRE(laserL.v.at(0).curve.b == Approx(0.4));
        REQUIRE(laserL.v.contains(240));
        REQUIRE(laserL.v.at(240).v.v == Approx(1.0));

        REQUIRE(chart.note.laser[1].contains(kMeasurePulse * 5 / 8));
        const auto& laserR = chart.note.laser[1].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserR.v.contains(0));
        REQUIRE(laserR.v.at(0).v.v == Approx(1.0));
        REQUIRE(laserR.v.at(0).curve.a == Approx(0.7));
        REQUIRE(laserR.v.at(0).curve.b == Approx(0.3));
        REQUIRE(laserR.v.contains(240));
        REQUIRE(laserR.v.at(240).v.v == Approx(0.0));
    }

    SECTION("Curve not applied to different pulse") {
        std::stringstream ss;
        ss << "title=Curve Test\n";
        ss << "artist=Test\n";
        ss << "effect=Test\n";
        ss << "jacket=\n";
        ss << "illustrator=\n";
        ss << "difficulty=challenge\n";
        ss << "level=1\n";
        ss << "t=120\n";
        ss << "ver=170\n";
        ss << "--\n";
        // Curve at different pulse should not be applied
        ss << "zoom_top_curve=0.3;0.7\n";
        ss << "0000|00|--\n";
        ss << "zoom_top=100\n";
        ss << "0000|00|--\n";
        ss << "zoom_bottom=50\n";
        ss << "0000|00|--\n";
        ss << "zoom_bottom_curve=0.4;0.6\n";
        ss << "zoom_side_curve=0.5;0.5\n";
        ss << "0000|00|--\n";
        ss << "--\n";
        ss << "zoom_side=-25\n";
        ss << "0000|00|--\n";
        ss << "center_split_curve=0.2;0.8\n";
        ss << "0000|00|--\n";
        ss << "center_split=200\n";
        ss << "0000|00|--\n";
        ss << "tilt_curve=0.1;0.9\n";
        ss << "laser_l_curve=0.6;0.4\n";
        ss << "0000|00|0-\n";
        ss << "0000|00|o-\n";
        ss << "tilt=0.1\n";
        ss << "laser_r_curve=0.7;0.3\n";
        ss << "0000|00|-0\n";
        ss << "0000|00|-o\n";
        ss << "--\n";

        kson::ChartData chart = kson::LoadKSHChartData(ss);
        REQUIRE(chart.error == kson::ErrorType::None);

        // zoom_top should NOT have curve (curve was at different pulse)
        REQUIRE(chart.camera.cam.body.zoomTop.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).curve.b == Approx(0.0));

        // zoom_bottom should NOT have curve (curve was at different pulse)
        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 2));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).curve.b == Approx(0.0));

        // zoom_side should NOT have curve (curve was at different pulse)
        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).curve.b == Approx(0.0));

        // center_split should NOT have curve (curve was at different pulse)
        const auto centerSplitItr = chart.camera.cam.body.centerSplit.begin();
        REQUIRE(centerSplitItr != chart.camera.cam.body.centerSplit.end());
        REQUIRE(centerSplitItr->second.v.v == Approx(200.0));
        REQUIRE(centerSplitItr->second.curve.a == Approx(0.0));
        REQUIRE(centerSplitItr->second.curve.b == Approx(0.0));

        // tilt should NOT have curve (curve was at different pulse)
        REQUIRE(chart.camera.tilt.manual.size() == 1);
        const auto& tiltSection = chart.camera.tilt.manual.begin()->second;
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.0));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.0));

        // laser L should NOT have curve at second point (curve was only for first point)
        REQUIRE(chart.note.laser[0].size() == 1);
        const auto& laserL = chart.note.laser[0].begin()->second;
        // First point should have curve
        REQUIRE(laserL.v.contains(0));
        REQUIRE(laserL.v.at(0).curve.a == Approx(0.6));
        REQUIRE(laserL.v.at(0).curve.b == Approx(0.4));
        // Second point should NOT have curve
        REQUIRE(laserL.v.size() >= 2);
        const auto secondPointItr = std::next(laserL.v.begin());
        REQUIRE(secondPointItr->second.curve.a == Approx(0.0));
        REQUIRE(secondPointItr->second.curve.b == Approx(0.0));

        // laser R should have curve (curve was at same pulse)
        REQUIRE(chart.note.laser[1].size() == 1);
        const auto& laserR = chart.note.laser[1].begin()->second;
        REQUIRE(laserR.v.contains(0));
        REQUIRE(laserR.v.at(0).curve.a == Approx(0.7));
        REQUIRE(laserR.v.at(0).curve.b == Approx(0.3));
    }
}

TEST_CASE("KSH scroll_speed Loading", "[ksh_io][scroll_speed]") {
	constexpr kson::Pulse kMeasurePulse = kson::kResolution4;

	SECTION("Simple scroll_speed changes") {
		std::stringstream ss;
		ss << "title=Scroll Speed Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "0000|00|--\n";
		ss << "scroll_speed=1.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "scroll_speed=2.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "scroll_speed=0.5\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		kson::ChartData chart = kson::LoadKSHChartData(ss);
		REQUIRE(chart.error == kson::ErrorType::None);

		// Check scroll_speed values
		REQUIRE(chart.beat.scrollSpeed.size() == 4);

		// Initial value (pulse 0)
		REQUIRE(chart.beat.scrollSpeed.contains(0));
		REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
		REQUIRE(chart.beat.scrollSpeed.at(0).v.vf == Approx(1.0));

		// First line in measure 0
		REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse / 4));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse / 4).v.v == Approx(1.0));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse / 4).v.vf == Approx(1.0));

		// Measure 1
		REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse).v.v == Approx(2.0));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse).v.vf == Approx(2.0));

		// Measure 2
		REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 2));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 2).v.v == Approx(0.5));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 2).v.vf == Approx(0.5));
	}

	SECTION("Immediate scroll_speed change") {
		std::stringstream ss;
		ss << "title=Scroll Speed Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "scroll_speed=1.0\n";
		ss << "scroll_speed=3.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		kson::ChartData chart = kson::LoadKSHChartData(ss);
		REQUIRE(chart.error == kson::ErrorType::None);

		// Check immediate change at pulse 360 (3/8 measure)
		REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 3 / 8));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.v == Approx(1.0));
		REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.vf == Approx(3.0));
	}
}

TEST_CASE("KSH Export round-trip", "[ksh_io][export][round_trip]") {
	auto testExportRoundTrip = [](const std::string& filename) {
		// Load original KSH
		auto chart1 = kson::LoadKSHChartData(filename);

		if (chart1.error != kson::ErrorType::None) {
			std::cerr << "Error loading KSH file: " << filename << std::endl;
			std::cerr << "Error code: " << static_cast<int>(chart1.error) << std::endl;
			std::cerr << "Warnings count: " << chart1.warnings.size() << std::endl;
			for (const auto& warning : chart1.warnings) {
				std::cerr << "  - " << warning << std::endl;
			}
		}
		REQUIRE(chart1.error == kson::ErrorType::None);

		// Export to KSH
		std::ostringstream oss1;
		const bool saveResult1 = kson::SaveKSHChartData(oss1, chart1);
		REQUIRE(saveResult1);
		std::string kshString1 = oss1.str();
		REQUIRE(!kshString1.empty());

		// Load exported KSH
		std::istringstream iss1(kshString1);
		auto chart2 = kson::LoadKSHChartData(iss1);
		REQUIRE(chart2.error == kson::ErrorType::None);

		// Compare basic properties
		REQUIRE(chart1.meta.title == chart2.meta.title);
		REQUIRE(chart1.meta.artist == chart2.meta.artist);
		REQUIRE(chart1.meta.chartAuthor == chart2.meta.chartAuthor);
		REQUIRE(chart1.meta.difficulty.idx == chart2.meta.difficulty.idx);
		REQUIRE(chart1.meta.level == chart2.meta.level);

		// Compare note counts
		for (int i = 0; i < kson::kNumBTLanes; ++i) {
			REQUIRE(chart1.note.bt[i].size() == chart2.note.bt[i].size());
		}
		for (int i = 0; i < kson::kNumFXLanes; ++i) {
			REQUIRE(chart1.note.fx[i].size() == chart2.note.fx[i].size());
		}
		for (int i = 0; i < kson::kNumLaserLanes; ++i) {
			REQUIRE(chart1.note.laser[i].size() == chart2.note.laser[i].size());
		}
	};

	SECTION("Gram[LT]") {
		testExportRoundTrip(g_assetsDir + "/Gram_lt.ksh");
	}

	SECTION("Gram[CH]") {
		testExportRoundTrip(g_assetsDir + "/Gram_ch.ksh");
	}

	SECTION("Gram[EX]") {
		testExportRoundTrip(g_assetsDir + "/Gram_ex.ksh");
	}

	SECTION("Gram[IN]") {
		testExportRoundTrip(g_assetsDir + "/Gram_in.ksh");
	}
}

TEST_CASE("KSH Export KSON round-trip", "[ksh_io][export][kson_round_trip]") {
	auto testKSONRoundTrip = [](const std::string& filename) {
		// ksh1 → kson1
		auto kson1 = kson::LoadKSHChartData(filename);
		REQUIRE(kson1.error == kson::ErrorType::None);
		INFO("Loaded KSH successfully: " << filename);

		// kson1 → ksh2
		INFO("Starting to save KSH: " << filename);
		std::ostringstream ossKsh;
		const bool saveKshResult = kson::SaveKSHChartData(ossKsh, kson1);
		REQUIRE(saveKshResult);
		INFO("Saved KSH successfully: " << filename);
		std::string kshString = ossKsh.str();
		REQUIRE(!kshString.empty());

		// ksh2 → kson2
		std::istringstream issKsh(kshString);
		auto kson2 = kson::LoadKSHChartData(issKsh);
		REQUIRE(kson2.error == kson::ErrorType::None);

		// Compare kson1 and kson2 as JSON
		std::ostringstream ossJson1;
		auto saveJson1Result = kson::SaveKSONChartData(ossJson1, kson1);
		REQUIRE(saveJson1Result == kson::ErrorType::None);
		std::string ksonString1 = ossJson1.str();
		REQUIRE(!ksonString1.empty());

		std::ostringstream ossJson2;
		auto saveJson2Result = kson::SaveKSONChartData(ossJson2, kson2);
		REQUIRE(saveJson2Result == kson::ErrorType::None);
		std::string ksonString2 = ossJson2.str();
		REQUIRE(!ksonString2.empty());

		nlohmann::json json1 = nlohmann::json::parse(ksonString1);
		nlohmann::json json2 = nlohmann::json::parse(ksonString2);

		REQUIRE(json1["format_version"] == json2["format_version"]);
		REQUIRE(json1["meta"] == json2["meta"]);
		REQUIRE(json1["beat"] == json2["beat"]);
		REQUIRE(json1["gauge"] == json2["gauge"]);
		REQUIRE(json1["note"] == json2["note"]);
		REQUIRE(json1["audio"] == json2["audio"]);
		REQUIRE(json1["camera"] == json2["camera"]);
		REQUIRE(json1["bg"] == json2["bg"]);

		if (json1.contains("editor") && json2.contains("editor"))
		{
			REQUIRE(json1["editor"] == json2["editor"]);
		}
		if (json1.contains("compat") && json2.contains("compat"))
		{
			// Override version before comparison
			json2["compat"]["ksh_version"] = json1["compat"]["ksh_version"];
			REQUIRE(json1["compat"] == json2["compat"]);
		}
		if (json1.contains("impl") && json2.contains("impl"))
		{
			REQUIRE(json1["impl"] == json2["impl"]);
		}
	};

	SECTION("Gram[LT]") {
		INFO("Testing file: " << g_assetsDir + "/Gram_lt.ksh");
		testKSONRoundTrip(g_assetsDir + "/Gram_lt.ksh");
	}

	SECTION("Gram[CH]") {
		INFO("Testing file: " << g_assetsDir + "/Gram_ch.ksh");
		testKSONRoundTrip(g_assetsDir + "/Gram_ch.ksh");
	}

	SECTION("Gram[EX]") {
		INFO("Testing file: " << g_assetsDir + "/Gram_ex.ksh");
		testKSONRoundTrip(g_assetsDir + "/Gram_ex.ksh");
	}

	SECTION("Gram[IN]") {
		INFO("Testing file: " << g_assetsDir + "/Gram_in.ksh");
		testKSONRoundTrip(g_assetsDir + "/Gram_in.ksh");
	}
}

TEST_CASE("KSH Export KSON round-trip (all songs)", "[.][ksh_io][export][kson_round_trip][all_songs]") {
	auto testKSONRoundTrip = [](const std::string& filename) {
		// ksh1 → kson1
		auto kson1 = kson::LoadKSHChartData(filename);

		if (kson1.error != kson::ErrorType::None) {
			std::cerr << "Error loading KSH file: " << filename << std::endl;
			std::cerr << "Error code: " << static_cast<int>(kson1.error) << std::endl;
			std::cerr << "Warnings count: " << kson1.warnings.size() << std::endl;
			for (const auto& warning : kson1.warnings) {
				std::cerr << "  - " << warning << std::endl;
			}
		}
		REQUIRE(kson1.error == kson::ErrorType::None);
		std::cerr << "Loaded KSH successfully: " << filename << std::endl;

		// Check skip list
		struct SkipEntry {
			const char* title;
			std::int32_t difficultyIdx; // 0=light, 1=challenge, 2=extended, 3=infinite, -1=all
			const char* reason;
		};

		constexpr std::array<SkipEntry, 21> kSkipList = {{
			{"Always Feel the Same", 2, "manual tilt values lose precision in KSH export"},
			{"\xCE\xB1steroiD", 2, "1/64 slams lose length info in KSH->KSON conversion"},
			{"Astrindjent", 3, "laser sections with small gaps cannot be preserved"},
			{"EVERLASTING HAPPiNESS\xE2\x86\x91", 2, "laser sections with small gaps cannot be preserved"},
			{"\xCE\xB5modec", 2, "laser sections with small gaps cannot be preserved"},
			{"Heliodor", 0, "closely spaced effects are lost in conversion"},
			{"Heliodor", 1, "closely spaced effects are lost in conversion"},
			{"Heliodor", 2, "closely spaced effects are lost in conversion"},
			{"Let's go Summer VACAtion!!!", -1, "empty effect name and closely spaced effects"},
			{"Orso Medium", -1, "closely spaced effect parameter changes are lost in conversion"},
			{"Prototype \xE3\x81\xBF\xE3\x82\x89\xE3\x81\x92", 1, "consecutive duplicate tilt scale values are optimized out"},
			{"RuiNAre", 2, "manual tilt values lose precision in KSH export"},
			{"Sister\xE2\x80\xA0Sister", -1, "closely spaced effect parameter changes are lost in conversion"},
			{"The Fourth Stage: Depression", -1, "closely spaced effect resets require note splitting"},
			{"Winter Planet", -1, "closely spaced effect parameter changes are lost in conversion"},
			{"YEARN \xEF\xBD\x9E\x44\x65\x76\x69\x6C\x69\x73\x68\x20\x52\x65\x64\x70\x69\x6E\x68\x65\x65\x6C\xEF\xBD\x9E", 3, "time signature in empty measure is lost"},
			{"\xE3\x81\xA8\xE3\x81\xA6\xE3\x82\x82\xE3\x81\x99\xE3\x81\xB0\xE3\x82\x89\xE3\x81\x97\xE3\x81\x84\x32\x30\x32\x30", 0, "tilt values clamped to valid range (±1000)"},
			{"\xE3\x83\x8F\xE3\x83\x94\xE3\x83\xA9\xE3\x82\xAD\xE2\x86\x92injection!!", -1, "laser sections with small gaps cannot be preserved"},
			{"\xE7\xB9\x9D\xE4\xB8\x8A\xE3\x83\xB4\xE7\xB9\x9D\xEF\xBD\xA9\xE7\xB9\xA7\xEF\xBD\xAD\xE7\xAB\x8A\xE6\xAE\xB5njection!!hogehogeaa", -1, "laser sections with small gaps (BOM-less UTF-8 encoding issue)"},
			{"\xD0\xA6\xD0\xB0\xD1\x80\xD1\x8C-\xD0\x9F\xD1\x83\xD1\x88\xD0\xBA\xD0\xB0", -1, "laser sections with small gaps cannot be preserved"},
			{"Zepter", 3, "closely spaced laser effect pulse events are lost in conversion"},
		}};

		for (const auto& skipEntry : kSkipList) {
			if (kson1.meta.title == skipEntry.title) {
				if (skipEntry.difficultyIdx == -1 || kson1.meta.difficulty.idx == skipEntry.difficultyIdx) {
					WARN("Skipping " << filename << " (" << skipEntry.reason << ")");
					return;
				}
			}
		}

		// kson1 → ksh2
		std::ostringstream ossKsh;
		const bool saveKshResult = kson::SaveKSHChartData(ossKsh, kson1);
		REQUIRE(saveKshResult);
		std::string kshString = ossKsh.str();
		REQUIRE(!kshString.empty());

		// ksh2 → kson2
		std::istringstream issKsh(kshString);
		auto kson2 = kson::LoadKSHChartData(issKsh);
		REQUIRE(kson2.error == kson::ErrorType::None);

		// Compare kson1 and kson2 as JSON
		std::ostringstream ossJson1;
		auto saveJson1Result = kson::SaveKSONChartData(ossJson1, kson1);
		REQUIRE(saveJson1Result == kson::ErrorType::None);
		std::string ksonString1 = ossJson1.str();
		REQUIRE(!ksonString1.empty());

		std::ostringstream ossJson2;
		auto saveJson2Result = kson::SaveKSONChartData(ossJson2, kson2);
		REQUIRE(saveJson2Result == kson::ErrorType::None);
		std::string ksonString2 = ossJson2.str();
		REQUIRE(!ksonString2.empty());

		nlohmann::json json1 = nlohmann::json::parse(ksonString1);
		nlohmann::json json2 = nlohmann::json::parse(ksonString2);

		// KSH→KSON→KSH→KSONなので、KSHで表現可能な情報は全て一致するはず
		INFO("File: " << filename);
		REQUIRE(json1["format_version"] == json2["format_version"]);
		REQUIRE(json1["meta"] == json2["meta"]);
		REQUIRE(json1["beat"] == json2["beat"]);
		REQUIRE(json1["gauge"] == json2["gauge"]);
		REQUIRE(json1["note"] == json2["note"]);
		REQUIRE(json1["audio"] == json2["audio"]);
		REQUIRE(json1["camera"] == json2["camera"]);
		REQUIRE(json1["bg"] == json2["bg"]);

		if (json1.contains("editor") && json2.contains("editor")) {
			REQUIRE(json1["editor"] == json2["editor"]);
		}
		if (json1.contains("compat") && json2.contains("compat")) {
			// Override version before comparison
			json2["compat"]["ksh_version"] = json1["compat"]["ksh_version"];
			REQUIRE(json1["compat"] == json2["compat"]);
		}
		if (json1.contains("impl") && json2.contains("impl")) {
			REQUIRE(json1["impl"] == json2["impl"]);
		}
	};

	std::filesystem::path songsPath = g_exeDir / "../../../kshootmania/App/songs";
	songsPath = std::filesystem::weakly_canonical(songsPath);
	if (!std::filesystem::exists(songsPath) || !std::filesystem::is_directory(songsPath)) {
		WARN("songs directory not found, skipping all songs test");
		return;
	}

	SECTION("All KSH files in songs directory") {
		std::vector<std::string> kshFiles;
		std::string searchCmd = "find \"" + songsPath.string() + "\" -name \"*.ksh\" 2>/dev/null";
		FILE* pipe = popen(searchCmd.c_str(), "r");
		if (pipe) {
			char buffer[1024];
			while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
				std::string filename = buffer;
				if (!filename.empty() && filename.back() == '\n') {
					filename.pop_back();
				}
				if (!filename.empty()) {
					kshFiles.push_back(filename);
				}
			}
			pclose(pipe);
		}

		if (kshFiles.empty()) {
			WARN("No KSH files found in songs directory");
			return;
		}

		for (const auto& file : kshFiles) {
			std::string relativePath = file;
			size_t songsPos = relativePath.find("/songs/");
			if (songsPos != std::string::npos) {
				relativePath = relativePath.substr(songsPos + 1);
			}

			INFO("Testing file: " << relativePath);
			testKSONRoundTrip(file);
		}
	}
}

TEST_CASE("KSH preset FX effect param_change export", "[ksh_io]")
{
	kson::ChartData chartData;
	chartData.meta.title = "Test FX ParamChange";
	chartData.meta.artist = "Test";
	chartData.meta.chartAuthor = "Test";
	chartData.meta.level = 1;
	chartData.meta.difficulty.idx = 0;
	chartData.beat.bpm[0] = 120.0;
	chartData.beat.timeSig[0] = kson::TimeSig{ .n = 4, .d = 4 };

	// Add preset FX effect param changes (KSON lowercase names)
	chartData.audio.audioEffect.fx.paramChange["retrigger"]["rate"].emplace(0, "100%");
	chartData.audio.audioEffect.fx.paramChange["retrigger"]["wave_length"].emplace(0, "1/8");
	chartData.audio.audioEffect.fx.paramChange["bitcrusher"]["mix"].emplace(0, "0%>50%");

	std::ostringstream oss;
	const bool result = kson::SaveKSHChartData(oss, chartData);
	REQUIRE(result);

	std::string kshContent = oss.str();
	INFO("KSH output:\n" << kshContent);

	// Should export with KSH preset names (PascalCase)
	REQUIRE(kshContent.find("fx:Retrigger:rate=100%") != std::string::npos);
	REQUIRE(kshContent.find("fx:Retrigger:waveLength=1/8") != std::string::npos);
	REQUIRE(kshContent.find("fx:BitCrusher:mix=0%>50%") != std::string::npos);

	// Should NOT export with KSON names (lowercase)
	REQUIRE(kshContent.find("fx:retrigger:") == std::string::npos);
	REQUIRE(kshContent.find("fx:bitcrusher:") == std::string::npos);
}

TEST_CASE("KSH preset FX effect param_change import", "[ksh_io]")
{
	std::string kshContent = R"(title=Test FX ParamChange
artist=Test
effect=Test
jacket=
illustrator=
difficulty=light
level=1
t=120
--
fx:Retrigger:rate=100%
fx:Retrigger:waveLength=1/8
fx:BitCrusher:mix=0%>50%
0000|00|--
--
)";

	std::istringstream iss(kshContent);
	auto chartData = kson::LoadKSHChartData(iss);
	REQUIRE(chartData.error == kson::ErrorType::None);

	// Should import with KSON names (lowercase)
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.contains("retrigger"));
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("retrigger").contains("rate"));
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("retrigger").at("rate").at(0) == "100%");
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("retrigger").contains("wave_length"));
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("retrigger").at("wave_length").at(0) == "1/8");

	REQUIRE(chartData.audio.audioEffect.fx.paramChange.contains("bitcrusher"));
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("bitcrusher").contains("mix"));
	REQUIRE(chartData.audio.audioEffect.fx.paramChange.at("bitcrusher").at("mix").at(0) == "0%>50%");

	// Should NOT import with KSH preset names (PascalCase)
	REQUIRE_FALSE(chartData.audio.audioEffect.fx.paramChange.contains("Retrigger"));
	REQUIRE_FALSE(chartData.audio.audioEffect.fx.paramChange.contains("BitCrusher"));
}

TEST_CASE("KSH preset laser filter param_change export", "[ksh_io]")
{
	kson::ChartData chartData;
	chartData.meta.title = "Test Filter ParamChange";
	chartData.meta.artist = "Test";
	chartData.meta.chartAuthor = "Test";
	chartData.meta.level = 1;
	chartData.meta.difficulty.idx = 0;
	chartData.beat.bpm[0] = 120.0;
	chartData.beat.timeSig[0] = kson::TimeSig{ .n = 4, .d = 4 };

	// Add preset laser filter param changes (KSON lowercase names)
	chartData.audio.audioEffect.laser.paramChange["peaking_filter"]["gain"].emplace(0, "60%");
	chartData.audio.audioEffect.laser.paramChange["high_pass_filter"]["freq"].emplace(0, "1000Hz");
	chartData.audio.audioEffect.laser.paramChange["low_pass_filter"]["freq"].emplace(0, "500Hz");
	chartData.audio.audioEffect.laser.paramChange["bitcrusher"]["mix"].emplace(0, "0%>50%");

	std::ostringstream oss;
	const bool result = kson::SaveKSHChartData(oss, chartData);
	REQUIRE(result);

	std::string kshContent = oss.str();
	INFO("KSH output:\n" << kshContent);

	// Should export with KSH preset filter names
	REQUIRE(kshContent.find("filter:peak:gain=60%") != std::string::npos);
	REQUIRE(kshContent.find("filter:hpf1:freq=1000Hz") != std::string::npos);
	REQUIRE(kshContent.find("filter:lpf1:freq=500Hz") != std::string::npos);
	REQUIRE(kshContent.find("filter:bitc:mix=0%>50%") != std::string::npos);

	// Should NOT export with KSON names
	REQUIRE(kshContent.find("filter:peaking_filter:") == std::string::npos);
	REQUIRE(kshContent.find("filter:high_pass_filter:") == std::string::npos);
	REQUIRE(kshContent.find("filter:low_pass_filter:") == std::string::npos);
	REQUIRE(kshContent.find("filter:bitcrusher:") == std::string::npos);

	// Should NOT export with short names without "1"
	REQUIRE(kshContent.find("filter:hpf:") == std::string::npos);
	REQUIRE(kshContent.find("filter:lpf:") == std::string::npos);
}

TEST_CASE("KSH preset laser filter param_change import", "[ksh_io]")
{
	std::string kshContent = R"(title=Test Filter ParamChange
artist=Test
effect=Test
jacket=
illustrator=
difficulty=light
level=1
t=120
--
filter:peak:gain=60%
filter:hpf1:freq=1000Hz
filter:lpf1:freq=500Hz
filter:bitc:mix=0%>50%
0000|00|--
--
)";

	std::istringstream iss(kshContent);
	auto chartData = kson::LoadKSHChartData(iss);
	REQUIRE(chartData.error == kson::ErrorType::None);

	// Should import with KSON names (lowercase with underscore)
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("peaking_filter"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("peaking_filter").contains("gain"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("peaking_filter").at("gain").at(0) == "60%");

	REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("high_pass_filter"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("high_pass_filter").contains("freq"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("high_pass_filter").at("freq").at(0) == "1000Hz");

	REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("low_pass_filter"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("low_pass_filter").contains("freq"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("low_pass_filter").at("freq").at(0) == "500Hz");

	REQUIRE(chartData.audio.audioEffect.laser.paramChange.contains("bitcrusher"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("bitcrusher").contains("mix"));
	REQUIRE(chartData.audio.audioEffect.laser.paramChange.at("bitcrusher").at("mix").at(0) == "0%>50%");

	// Should NOT import with KSH preset filter names
	REQUIRE_FALSE(chartData.audio.audioEffect.laser.paramChange.contains("peak"));
	REQUIRE_FALSE(chartData.audio.audioEffect.laser.paramChange.contains("hpf1"));
	REQUIRE_FALSE(chartData.audio.audioEffect.laser.paramChange.contains("lpf1"));
	REQUIRE_FALSE(chartData.audio.audioEffect.laser.paramChange.contains("bitc"));
}

