#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/IO/KshIO.hpp>
#include <kson/IO/KsonIO.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

extern std::string g_assetsDir;
extern std::filesystem::path g_exeDir;

TEST_CASE("KSON I/O lossless test (bundled charts)", "[kson_io][kson_lossless][bundled]") {
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

TEST_CASE("Gram[EX] detailed chart validation", "[ksh_io][bundled]") {
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
		REQUIRE(chart.camera.tilt.size() == 16);
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

TEST_CASE("KSON I/O lossless test (all songs)", "[.][kson_io][kson_lossless][all_songs]") {
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

    // Check legacy.filter_gain
    REQUIRE(!chartData.audio.audioEffect.laser.legacy.filterGain.empty());
    const auto& pfiltergain = chartData.audio.audioEffect.laser.legacy.filterGain;

    // pfiltergain=0 should convert to 0.0
    REQUIRE(pfiltergain.contains(0));
    REQUIRE(std::abs(pfiltergain.at(0) - 0.0) < 0.001);

    // pfiltergain=50 should convert to 0.5
    REQUIRE(pfiltergain.contains(kson::kResolution4));
    REQUIRE(std::abs(pfiltergain.at(kson::kResolution4) - 0.5) < 0.001);

    // pfiltergain=100 should convert to 1.0
    REQUIRE(pfiltergain.contains(kson::kResolution4 * 2));
    REQUIRE(std::abs(pfiltergain.at(kson::kResolution4 * 2) - 1.0) < 0.001);
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

        REQUIRE(chart.camera.tilt.contains(kMeasurePulse / 2));
        const auto& tiltValue = chart.camera.tilt.at(kMeasurePulse / 2);
        REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue));
        const auto& tiltGraphPoint = std::get<kson::TiltGraphPoint>(tiltValue);
        REQUIRE(tiltGraphPoint.v.v == Approx(0.1));
        REQUIRE(std::holds_alternative<double>(tiltGraphPoint.v.vf));
        REQUIRE(std::get<double>(tiltGraphPoint.v.vf) == Approx(0.1));

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

        REQUIRE(chart.camera.tilt.contains(kMeasurePulse / 2));
        const auto& tiltValue = chart.camera.tilt.at(kMeasurePulse / 2);
        REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue));
        const auto& tiltGraphPoint = std::get<kson::TiltGraphPoint>(tiltValue);
        REQUIRE(tiltGraphPoint.v.v == Approx(0.1));
        REQUIRE(std::holds_alternative<double>(tiltGraphPoint.v.vf));
        REQUIRE(std::get<double>(tiltGraphPoint.v.vf) == Approx(0.1));

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
        REQUIRE(chart.camera.tilt.size() == 1);
        const auto& tiltValue = chart.camera.tilt.begin()->second;
        REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue));
        const auto& tiltGraphPoint = std::get<kson::TiltGraphPoint>(tiltValue);
        REQUIRE(tiltGraphPoint.v.v == Approx(0.1));
        REQUIRE(std::holds_alternative<double>(tiltGraphPoint.v.vf));
        REQUIRE(std::get<double>(tiltGraphPoint.v.vf) == Approx(0.1));

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

TEST_CASE("KSH I/O lossless test (bundled charts)", "[ksh_io][kson_io][ksh_lossless][bundled]") {
	auto testKSONRoundTrip = [](const std::string& filename) {
		// ksh1 → kson1
		auto kson1 = kson::LoadKSHChartData(filename);
		REQUIRE(kson1.error == kson::ErrorType::None);
		INFO("Loaded KSH successfully: " << filename);

		// kson1 → ksh2
		INFO("Starting to save KSH: " << filename);
		std::ostringstream ossKsh;
		const kson::ErrorType saveKshResult = kson::SaveKSHChartData(ossKsh, kson1);
		REQUIRE(saveKshResult == kson::ErrorType::None);
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

TEST_CASE("KSH I/O lossless test (all songs)", "[.][ksh_io][kson_io][ksh_lossless][all_songs]") {
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

		// kson1 → ksh2
		std::ostringstream ossKsh;
		const kson::ErrorType saveKshResult = kson::SaveKSHChartData(ossKsh, kson1);
		REQUIRE(saveKshResult == kson::ErrorType::None);
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

		// KSH->KSON->KSH->KSON, so all lines representable in KSH should match
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

// Common function for KSH round-trip tests on song directories
static void RunKSHRoundTripTest(
	const std::string& testName,
	const std::string& dirName,
	const std::set<std::string>& knownFailures)
{
	// Filter KSH lines for comparison
	auto filterKSHLines = [](const std::string& kshContent) -> std::vector<std::string> {
		std::vector<std::string> lines;
		std::istringstream iss(kshContent);
		std::string line;
		while (std::getline(iss, line)) {
			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}
			lines.push_back(line);
		}

		std::vector<std::string> filtered;
		const std::vector<std::string> ignorePrefixes = {
			"//", "title=", "title_img=", "artist=", "artist_img=", "effect=",
			"illustrator=", "jacket=", "level=", "difficulty=", "m=", "o=", "v=",
			"vo=", "mvol=", "bg=", "layer=", "po=", "plength=", "pfiltergain=",
			"filtertype=", "chokkakuautovol=", "chokkakuvol=", "chokkakuse=",
			"icon=", "ver=", "fx-l=", "fx-r=", "#define", "information=",
			"filter:", "fx:",
		};

		for (auto line : lines) {
			std::string stripped = line;
			size_t start = stripped.find_first_not_of(" \t\r\n");
			if (start == std::string::npos) {
				stripped = "";
			} else {
				size_t end = stripped.find_last_not_of(" \t\r\n");
				stripped = stripped.substr(start, end - start + 1);
			}

			bool shouldIgnore = false;
			for (const auto& prefix : ignorePrefixes) {
				if (stripped.rfind(prefix, 0) == 0) {
					shouldIgnore = true;
					break;
				}
			}
			if (shouldIgnore) continue;

			// Normalize fx-l_se/fx-r_se values
			if (line.find("fx-l_se=") != std::string::npos || line.find("fx-r_se=") != std::string::npos) {
				size_t eqPos = line.find('=');
				if (eqPos != std::string::npos) {
					std::string keyPart = line.substr(0, eqPos);
					std::string valuePart = line.substr(eqPos + 1);
					std::vector<std::string> elements;
					size_t pos = 0;
					while (pos < valuePart.size()) {
						size_t nextPos = valuePart.find(';', pos);
						if (nextPos == std::string::npos) {
							elements.push_back(valuePart.substr(pos));
							break;
						}
						elements.push_back(valuePart.substr(pos, nextPos - pos));
						pos = nextPos + 1;
					}
					if (elements.size() > 2) {
						elements.resize(2);
					}
					if (elements.size() == 2 && elements[1] == "100") {
						elements.resize(1);
					}
					std::string normalizedValue;
					for (size_t i = 0; i < elements.size(); ++i) {
						if (i > 0) normalizedValue += ";";
						normalizedValue += elements[i];
					}
					line = keyPart + "=" + normalizedValue;
				}
			}

			filtered.push_back(line);
		}

		// Remove trailing empty measures
		while (filtered.size() >= 2) {
			std::string last = filtered.back();
			std::string secondLast = filtered[filtered.size() - 2];
			size_t s1 = last.find_first_not_of(" \t\r\n");
			if (s1 != std::string::npos) {
				size_t e1 = last.find_last_not_of(" \t\r\n");
				last = last.substr(s1, e1 - s1 + 1);
			}
			size_t s2 = secondLast.find_first_not_of(" \t\r\n");
			if (s2 != std::string::npos) {
				size_t e2 = secondLast.find_last_not_of(" \t\r\n");
				secondLast = secondLast.substr(s2, e2 - s2 + 1);
			}
			if (last == "--" && secondLast == "0000|00|--") {
				filtered.pop_back();
				filtered.pop_back();
			} else {
				break;
			}
		}

		return filtered;
	};

	auto testKSHRoundTrip = [&filterKSHLines](const std::string& filename) -> bool {
		std::ifstream originalFile(filename);
		if (!originalFile) return false;
		std::string originalContent((std::istreambuf_iterator<char>(originalFile)),
		                            std::istreambuf_iterator<char>());

		// ksh1 -> kson
		auto kson1 = kson::LoadKSHChartData(filename);
		if (kson1.error != kson::ErrorType::None) {
			return false;
		}

		// kson -> ksh2
		std::ostringstream ossKsh;
		const kson::ErrorType saveKshResult = kson::SaveKSHChartData(ossKsh, kson1);
		if (saveKshResult != kson::ErrorType::None) {
			return false;
		}
		std::string kshString = ossKsh.str();
		if (kshString.empty()) {
			return false;
		}

		// Compare ksh1 and ksh2 (filtered)
		auto originalFiltered = filterKSHLines(originalContent);
		auto resultFiltered = filterKSHLines(kshString);

		return originalFiltered == resultFiltered;
	};

	std::filesystem::path targetPath = g_exeDir / ("../../../kshootmania/App/songs/" + dirName);
	targetPath = std::filesystem::weakly_canonical(targetPath);
	if (!std::filesystem::exists(targetPath) || !std::filesystem::is_directory(targetPath)) {
		WARN(dirName + " directory not found, skipping test");
		return;
	}

	std::vector<std::string> kshFiles;
	std::string searchCmd = "find \"" + targetPath.string() + "\" -name \"*.ksh\" 2>/dev/null";
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
		WARN("No KSH files found in " + dirName + " directory");
		return;
	}

	int passed = 0;
	int failed = 0;
	int knownFailureCount = 0;
	std::vector<std::string> unexpectedFailures;
	std::vector<std::string> unexpectedSuccesses;

	for (const auto& file : kshFiles) {
		// Extract relative path from target directory
		std::string relativePath = file;
		size_t dirPos = relativePath.find("/" + dirName + "/");
		if (dirPos != std::string::npos) {
			relativePath = relativePath.substr(dirPos + dirName.length() + 2);
		}

		bool success = false;
		try {
			success = testKSHRoundTrip(file);
		} catch (const std::exception& e) {
			success = false;
		}

		if (success) {
			passed++;
			if (knownFailures.count(relativePath) > 0) {
				unexpectedSuccesses.push_back(relativePath);
			}
		} else {
			failed++;
			if (knownFailures.count(relativePath) == 0) {
				unexpectedFailures.push_back(relativePath);
			} else {
				knownFailureCount++;
			}
		}
	}

	int total = passed + failed;
	int totalExcludingKnownFailures = total - knownFailureCount;
	int passedExcludingKnownFailures = passed + unexpectedSuccesses.size();
	double successRateExcludingKnown = totalExcludingKnownFailures > 0
		? (100.0 * passedExcludingKnownFailures / totalExcludingKnownFailures)
		: 0.0;
	double successRate = total > 0 ? (100.0 * passed / total) : 0.0;

	bool testSuccess = unexpectedFailures.empty();
	std::cerr << "\n=== " << testName << " Round-Trip Test: " << (testSuccess ? "SUCCESS" : "FAILURE") << " ===" << std::endl;
	std::cerr << passedExcludingKnownFailures << "/" << totalExcludingKnownFailures
		<< " (" << static_cast<int>(successRateExcludingKnown) << "%)" << std::endl;
	std::cerr << "  - Total: " << passed << "/" << total << " (" << static_cast<int>(successRate) << "%)" << std::endl;
	std::cerr << "  - Known failures: " << knownFailureCount << std::endl;
	std::cerr << "  - New failures: " << unexpectedFailures.size() << std::endl;

	if (!unexpectedSuccesses.empty()) {
		std::cerr << "\nUnexpected successes (remove from known failures list):" << std::endl;
		for (const auto& file : unexpectedSuccesses) {
			std::cerr << "  - " << file << std::endl;
		}
	}

	if (!unexpectedFailures.empty()) {
		std::cerr << "\nUnexpected failures (NEW REGRESSIONS):" << std::endl;
		for (const auto& file : unexpectedFailures) {
			std::cerr << "  - " << file << std::endl;
		}
		REQUIRE(unexpectedFailures.empty());
	}
}

TEST_CASE("KSH I/O round-trip test (SFES2022)", "[.][ksh_io][kson_io][sfes2022]") {
	// Known failing files (relative paths from songs/SFES2022/)
	const std::set<std::string> knownFailures = {
		"024/1_Turbulence/4_in.ksh",
		"024/1_Turbulence/2_ch.ksh",
		"024/1_Turbulence/1_lt.ksh",
		"024/3_Prototype_Mirage/4_in.ksh",
		"024/3_Prototype_Mirage/2_ch.ksh",
		"024/3_Prototype_Mirage/3_ex.ksh",
		"024/3_Prototype_Mirage/1_lt.ksh",
		"023/Externus/in.ksh",
		"015/Splash/Splash_ADV.ksh",
		"015/Splash/Splash_EXH.ksh",
		"012/pessimism/pessimism_IN.ksh",
		"013/RuiNAre/EX.ksh",
		"013/the_oscillation/IN.ksh",
		"013/the_oscillation/EX.ksh",
		"013/always_feel_the_same/EX.ksh",
		"008/03_R_D/R_D_in.ksh",
		"001/Diana/Diana_EX.ksh",
		"001/Indecisive/Indecisive_IN.ksh",
		"006/miasmatheory/miasma1.ksh",
		"006/orphen/orphen2.ksh",
		"006/lamentia/lamentia2.ksh",
		"020/liberAge/liberage_n.ksh",
		"018/Lagurus/Lagurus_IN.ksh",
		"018/Lagurus/Lagurus_EX.ksh",
		"018/Half_Moon/Half_Moon_EX.ksh",
		"011/CASE1_Missing_Zeppelin/EX.ksh",
		"011/I_believe_in_RAW_STYLE/IN.ksh",
		"029/Witchs_Curse/1_lt.ksh",
		"010/desire drive_Electro_Swing_Remix/EX.ksh",
		"010/Autumn_Fire/IN.ksh",
		"019/2_bad_night/4_in.ksh",
		"021/01_BulbBubble/IN.ksh",
		"032/over/lt.ksh",
		"032/over/ch.ksh",
		"002/2himitunorojiuranightdisco/IN.ksh",
	};

	RunKSHRoundTripTest("SFES2022", "SFES2022", knownFailures);
}

TEST_CASE("KSH I/O round-trip test (SFES2020)", "[.][ksh_io][kson_io][sfes2020]") {
	// Known failing files (relative paths from songs/SFES2020/)
	const std::set<std::string> knownFailures = {
		"046/03/inf.ksh",
		"013/02_9318054/9318054_ex.ksh",
		"022/03_murderdoll/murderdoll_ex.ksh",
		"022/02_avesta/avesta_in.ksh",
		"025/Ruten/Rut_ex.ksh",
		"025/The_Rats_in_the_Walls/Rats_in.ksh",
		"025/70_Minutes_Fighters/70m_ch.ksh",
		"025/70_Minutes_Fighters/70m_lt.ksh",
		"025/70_Minutes_Fighters/70m_in.ksh",
		"025/70_Minutes_Fighters/70m_ex.ksh",
		"049/2_picopicotmg/picopicotmg_kiritan_tohu.ksh",
		"049/3_tamago_tetsuaji/tamago_tetsuaji_tmmgdn.ksh",
		"049/3_tamago_tetsuaji/tamago_tetsuaji_india.ksh",
		"049/3_tamago_tetsuaji/tamago_tetsuaji_mokyu.ksh",
		"049/1_tamago/tamago_ogamat.ksh",
		"040/3_starrydarkbluesky/4_in.ksh",
		"037/MoVe/MoVe_INF.ksh",
		"039/Addict/IN.ksh",
		"027/PlanetProbe/LT.ksh",
		"018/VectorZero/In.ksh",
		"044/Neva_eh/Neva_eh_4.ksh",
		"044/Neva_eh/Neva_eh_3.ksh",
		"043/2_goodbye_katsura/2_ch.ksh",
		"043/2_goodbye_katsura/3_ex.ksh",
		"043/3_emodec/2_ch.ksh",
		"043/3_emodec/3_ex.ksh",
		"043/3_emodec/1_lt.ksh",
		"017/3_Nightmares/Nightmares_EX.ksh",
		"017/3_Nightmares/Nightmares_IN.ksh",
		"017/3_Nightmares/Nightmares_CH.ksh",
		"017/2_AllWeponsFullDrive/AllWeponsFullDrive_in.ksh",
		"028/03_Momonjes_Defilee/IN.ksh",
		"010/phonautograph/IN.ksh",
		"026/02_mini_lonesomewarcry/lonesome_warcry_lt.ksh",
		"026/01_mini_cosmiclightparade/cosmic_lightparade_ex.ksh",
		"019/Executor/nov.ksh",
		"019/Executor/mxm.ksh",
		"019/Executor/exh.ksh",
		"019/Executor/adv.ksh",
		"035/shou/1_LT.ksh",
		"034/1_kikagaku/kikagaku_lt.ksh",
		"034/1_kikagaku/kikagaku_ch.ksh",
		"034/1_kikagaku/kikagaku_in.ksh",
		"034/2_sakura_sakura/sakura_sakura_in.ksh",
		"034/2_sakura_sakura/sakura_sakura_ex.ksh",
		"034/2_sakura_sakura/sakura_sakura_ch.ksh",
	};

	RunKSHRoundTripTest("SFES2020", "SFES2020", knownFailures);
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
	const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
	REQUIRE(result == kson::ErrorType::None);

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
	const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
	REQUIRE(result == kson::ErrorType::None);

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

TEST_CASE("KSH version field preservation", "[ksh_io][version]")
{
	SECTION("ver >= 160 is preserved") {
		std::string kshContent = R"(title=Test
artist=Test
effect=Test
jacket=
illustrator=
difficulty=light
level=1
t=120
ver=170
--
0000|00|--
--
)";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);
		REQUIRE(chartData.compat.kshVersion == "170");

		std::ostringstream oss;
		const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();
		REQUIRE(kshOutput.find("ver=170") != std::string::npos);
	}

	SECTION("ver < 160 is upgraded to 160") {
		std::string kshContent = R"(title=Test
artist=Test
effect=Test
jacket=
illustrator=
difficulty=light
level=1
t=120
ver=130
--
0000|00|--
--
)";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);
		REQUIRE(chartData.compat.kshVersion == "130");

		std::ostringstream oss;
		const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();
		REQUIRE(kshOutput.find("ver=160") != std::string::npos);
		REQUIRE(kshOutput.find("ver=130") == std::string::npos);
	}
}

TEST_CASE("ver_compat output for legacy versions", "[ksh_io][ver_compat]") {
	SECTION("ver < 160 should output ver_compat") {
		// Create minimal KSH with ver=130
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=120\n"
			"m=test.ogg\n"
			"ver=130\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);
		REQUIRE(chartData.compat.kshVersion == "130");

		// Convert to KSH
		std::ostringstream oss;
		const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();

		// Should output ver=160
		REQUIRE(kshOutput.find("ver=160") != std::string::npos);

		// Should also output ver_compat=130
		REQUIRE(kshOutput.find("ver_compat=130") != std::string::npos);

		// ver_compat should come after ver=
		size_t verPos = kshOutput.find("ver=160");
		size_t verCompatPos = kshOutput.find("ver_compat=130");
		REQUIRE(verCompatPos > verPos);
	}

	SECTION("ver >= 160 should not output ver_compat") {
		// Create minimal KSH with ver=171
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=120\n"
			"m=test.ogg\n"
			"ver=171\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);
		REQUIRE(chartData.compat.kshVersion == "171");

		// Convert to KSH
		std::ostringstream oss;
		const kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();

		// Should output ver=171
		REQUIRE(kshOutput.find("ver=171") != std::string::npos);

		// Should NOT output ver_compat
		REQUIRE(kshOutput.find("ver_compat") == std::string::npos);
	}
}

TEST_CASE("ver_compat reading", "[ksh_io][ver_compat]") {
	SECTION("Reading KSH with ver_compat should use ver_compat for kshVersion") {
		// KSH file with both ver=160 and ver_compat=130
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=120\n"
			"m=test.ogg\n"
			"ver=160\n"
			"ver_compat=130\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// Should use ver_compat=130, not ver=160
		REQUIRE(chartData.compat.kshVersion == "130");
	}

	SECTION("Reading KSH without ver_compat should use ver for kshVersion") {
		// KSH file with only ver=160
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=120\n"
			"m=test.ogg\n"
			"ver=160\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// Should use ver=160
		REQUIRE(chartData.compat.kshVersion == "160");
	}
}

TEST_CASE("BPM limit for ver >= 130", "[ksh_io][bpm_limit]") {
	SECTION("ver >= 130 should limit BPM to 65535.0") {
		// KSH file with ver=130 and BPM > 65535
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=100000\n"
			"m=test.ogg\n"
			"ver=130\n"
			"--\n"
			"t=80000\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// BPM should be clamped to 65535.0
		REQUIRE(chartData.beat.bpm.at(0) == Approx(65535.0));
		REQUIRE(chartData.beat.bpm.at(0) == Approx(65535.0));
	}

	SECTION("ver < 130 should not limit BPM") {
		// KSH file with ver=120 and BPM > 65535
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=100000\n"
			"m=test.ogg\n"
			"ver=120\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// BPM should NOT be clamped
		REQUIRE(chartData.beat.bpm.at(0) == Approx(100000.0));
	}

	SECTION("ver >= 160 should still limit BPM to 65535.0") {
		// KSH file with ver=171 and BPM > 65535
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=100000\n"
			"m=test.ogg\n"
			"ver=171\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// BPM should be clamped to 65535.0
		REQUIRE(chartData.beat.bpm.at(0) == Approx(65535.0));
	}

	SECTION("ver_compat should be used for BPM limit check") {
		// KSH file with ver=160, ver_compat=120 and BPM > 65535
		std::string kshContent =
			"title=Test\n"
			"artist=Test\n"
			"effect=Test\n"
			"jacket=nowprinting1\n"
			"illustrator=\n"
			"difficulty=light\n"
			"level=1\n"
			"t=100000\n"
			"m=test.ogg\n"
			"ver=160\n"
			"ver_compat=120\n"
			"--\n"
			"0000|00|--\n"
			"--\n";

		std::istringstream iss(kshContent);
		auto chartData = kson::LoadKSHChartData(iss);
		REQUIRE(chartData.error == kson::ErrorType::None);

		// BPM should NOT be clamped because ver_compat=120 < 130
		REQUIRE(chartData.beat.bpm.at(0) == Approx(100000.0));
	}
}

TEST_CASE("BPM clamping on KSH output", "[ksh_io][bpm_output]") {
	SECTION("ver >= 130 should clamp BPM on output") {
		// Create chart data with ver=130 and BPM > 65535
		kson::ChartData chartData;
		chartData.meta.title = "Test";
		chartData.meta.artist = "Test";
		chartData.meta.chartAuthor = "Test";
		chartData.meta.jacketFilename = "nowprinting1";
		chartData.meta.difficulty.idx = 0;
		chartData.meta.level = 1;
		chartData.audio.bgm.filename = "test.ogg";
		chartData.beat.bpm[0] = 100000.0;
		chartData.beat.bpm[192] = 80000.0;
		chartData.beat.timeSig[0] = {4, 4};
		chartData.compat.kshVersion = "130";

		// Convert to KSH
		std::ostringstream oss;
		kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();

		// BPM should be clamped to 65535.0
		REQUIRE(kshOutput.find("t=65535") != std::string::npos);
		REQUIRE(kshOutput.find("100000") == std::string::npos);
		REQUIRE(kshOutput.find("80000") == std::string::npos);
	}

	SECTION("ver < 130 should not clamp BPM on output") {
		// Create chart data with ver=120 and BPM > 65535
		kson::ChartData chartData;
		chartData.meta.title = "Test";
		chartData.meta.artist = "Test";
		chartData.meta.chartAuthor = "Test";
		chartData.meta.jacketFilename = "nowprinting1";
		chartData.meta.difficulty.idx = 0;
		chartData.meta.level = 1;
		chartData.audio.bgm.filename = "test.ogg";
		chartData.beat.bpm[0] = 100000.0;
		chartData.beat.timeSig[0] = {4, 4};
		chartData.compat.kshVersion = "120";

		// Convert to KSH
		std::ostringstream oss;
		kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();

		// BPM should NOT be clamped
		REQUIRE(kshOutput.find("t=100000") != std::string::npos);
	}

	SECTION("Multiple BPM values should be clamped if needed") {
		// Create chart data with multiple BPMs (some over limit, some under)
		kson::ChartData chartData;
		chartData.meta.title = "Test";
		chartData.meta.artist = "Test";
		chartData.meta.chartAuthor = "Test";
		chartData.meta.jacketFilename = "nowprinting1";
		chartData.meta.difficulty.idx = 0;
		chartData.meta.level = 1;
		chartData.audio.bgm.filename = "test.ogg";
		chartData.beat.bpm[0] = 100000.0;
		chartData.beat.bpm[192] = 70000.0;
		chartData.beat.bpm[384] = 50000.0;
		chartData.beat.timeSig[0] = {4, 4};
		chartData.compat.kshVersion = "160";

		// Convert to KSH
		std::ostringstream oss;
		kson::ErrorType result = kson::SaveKSHChartData(oss, chartData);
		REQUIRE(result == kson::ErrorType::None);

		std::string kshOutput = oss.str();

		// BPMs over 65535 should be clamped
		REQUIRE(kshOutput.find("65535") != std::string::npos);
		REQUIRE(kshOutput.find("100000") == std::string::npos);
		REQUIRE(kshOutput.find("70000") == std::string::npos);
		// 50000 is under the limit, so it should remain as-is
		REQUIRE(kshOutput.find("50000") != std::string::npos);
	}
}

TEST_CASE("KSH bg output deduplication", "[ksh_io][bg]") {
	kson::ChartData chartData;
	chartData.meta.title = "test";

	SECTION("Different bg filenames outputs both") {
		chartData.bg.legacy.bg[0].filename = "bg1";
		chartData.bg.legacy.bg[1].filename = "bg2";
		std::ostringstream oss;
		kson::SaveKSHChartData(oss, chartData);
		REQUIRE(oss.str().find("bg=bg1;bg2") != std::string::npos);
	}

	SECTION("Same bg filenames outputs only one") {
		chartData.bg.legacy.bg[0].filename = "bg1";
		chartData.bg.legacy.bg[1].filename = "bg1";
		std::ostringstream oss;
		kson::SaveKSHChartData(oss, chartData);
		REQUIRE(oss.str().find("bg=bg1\r\n") != std::string::npos);
	}
}

TEST_CASE("KSH Manual Tilt with Curve", "[ksh_io][tilt][curve]") {
	constexpr kson::Pulse kMeasurePulse = kson::kResolution4;

	SECTION("Manual tilt with curve interpolation") {
		std::stringstream ss;
		ss << "title=Manual Tilt Curve Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=0.0\n";
		ss << "tilt_curve=0.1;0.5\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=5.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// First tilt point should have curve
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& tiltValue0 = chart.camera.tilt.at(0);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue0));
		const auto& point0 = std::get<kson::TiltGraphPoint>(tiltValue0);
		REQUIRE(point0.v.v == Approx(0.0));
		REQUIRE(std::holds_alternative<double>(point0.v.vf));
		REQUIRE(std::get<double>(point0.v.vf) == Approx(0.0));
		REQUIRE(point0.curve.a == Approx(0.1));
		REQUIRE(point0.curve.b == Approx(0.5));

		// Second tilt point should not have curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& tiltValue1 = chart.camera.tilt.at(kMeasurePulse);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue1));
		const auto& point1 = std::get<kson::TiltGraphPoint>(tiltValue1);
		REQUIRE(point1.v.v == Approx(5.0));
		REQUIRE(std::holds_alternative<double>(point1.v.vf));
		REQUIRE(std::get<double>(point1.v.vf) == Approx(5.0));
		REQUIRE(point1.curve.isLinear());
	}

	SECTION("Manual tilt with immediate change and curve") {
		std::stringstream ss;
		ss << "title=Manual Tilt Immediate Change Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt_curve=0.3;0.7\n";
		ss << "tilt=0.0\n";
		ss << "tilt=2.5\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=8.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// Immediate change should preserve curve
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& tiltValue = chart.camera.tilt.at(0);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue));
		const auto& point = std::get<kson::TiltGraphPoint>(tiltValue);
		REQUIRE(point.v.v == Approx(0.0));  // First value
		REQUIRE(std::holds_alternative<double>(point.v.vf));
		REQUIRE(std::get<double>(point.v.vf) == Approx(2.5)); // Second value (immediate change)
		REQUIRE(point.curve.a == Approx(0.3)); // Curve should be preserved
		REQUIRE(point.curve.b == Approx(0.7));
	}

	SECTION("Manual tilt with curve between immediate change values") {
		std::stringstream ss;
		ss << "title=Manual Tilt Curve Between Values Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=0.0\n";
		ss << "tilt_curve=0.5;0.5\n";
		ss << "tilt=3.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=6.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// Curve between first and second tilt should be preserved in immediate change
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& tiltValue = chart.camera.tilt.at(0);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue));
		const auto& point = std::get<kson::TiltGraphPoint>(tiltValue);
		REQUIRE(point.v.v == Approx(0.0));  // First value
		REQUIRE(std::holds_alternative<double>(point.v.vf));
		REQUIRE(std::get<double>(point.v.vf) == Approx(3.0)); // Second value (immediate change)
		REQUIRE(point.curve.a == Approx(0.5)); // Curve should be preserved
		REQUIRE(point.curve.b == Approx(0.5));

		// Next tilt point
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& point2 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse));
		REQUIRE(point2.v.v == Approx(6.0));
		REQUIRE(point2.curve.isLinear());
	}

	SECTION("Manual tilt with multiple curves and immediate changes") {
		std::stringstream ss;
		ss << "title=Complex Manual Tilt Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=0.0\n";
		ss << "tilt_curve=0.2;0.8\n";
		ss << "tilt=1.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=3.0\n";
		ss << "tilt_curve=0.6;0.4\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt_curve=0.7;0.3\n";
		ss << "tilt=5.0\n";
		ss << "tilt=7.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// First immediate change with curve
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& point0 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(0));
		REQUIRE(point0.v.v == Approx(0.0));
		REQUIRE(std::holds_alternative<double>(point0.v.vf));
		REQUIRE(std::get<double>(point0.v.vf) == Approx(1.0));
		REQUIRE(point0.curve.a == Approx(0.2));
		REQUIRE(point0.curve.b == Approx(0.8));

		// Second point with curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& point1 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse));
		REQUIRE(point1.v.v == Approx(3.0));
		REQUIRE(point1.curve.a == Approx(0.6));
		REQUIRE(point1.curve.b == Approx(0.4));

		// Third immediate change with curve before both values
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 2));
		const auto& point2 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse * 2));
		REQUIRE(point2.v.v == Approx(5.0));
		REQUIRE(std::holds_alternative<double>(point2.v.vf));
		REQUIRE(std::get<double>(point2.v.vf) == Approx(7.0));
		REQUIRE(point2.curve.a == Approx(0.7));
		REQUIRE(point2.curve.b == Approx(0.3));
	}

	SECTION("Manual tilt with multiple curves") {
		std::stringstream ss;
		ss << "title=Manual Tilt Multiple Curves Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=0.0\n";
		ss << "tilt_curve=0.2;0.8\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=3.0\n";
		ss << "tilt_curve=0.6;0.4\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=7.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// First point with first curve
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& point0 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(0));
		REQUIRE(point0.v.v == Approx(0.0));
		REQUIRE(point0.curve.a == Approx(0.2));
		REQUIRE(point0.curve.b == Approx(0.8));

		// Second point with second curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& point1 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse));
		REQUIRE(point1.v.v == Approx(3.0));
		REQUIRE(point1.curve.a == Approx(0.6));
		REQUIRE(point1.curve.b == Approx(0.4));

		// Third point without curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 2));
		const auto& point2 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse * 2));
		REQUIRE(point2.v.v == Approx(7.0));
		REQUIRE(point2.curve.isLinear());
	}

	SECTION("Mix of auto tilt and manual tilt with curve") {
		std::stringstream ss;
		ss << "title=Mixed Tilt Test\n";
		ss << "artist=Test\n";
		ss << "effect=Test\n";
		ss << "jacket=nowprinting1\n";
		ss << "illustrator=Test\n";
		ss << "difficulty=challenge\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=normal\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=1.5\n";
		ss << "tilt_curve=0.4;0.6\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=4.0\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=bigger\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// Auto tilt (normal)
		REQUIRE(chart.camera.tilt.contains(0));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(chart.camera.tilt.at(0)));
		REQUIRE(std::get<kson::AutoTiltType>(chart.camera.tilt.at(0)) == kson::AutoTiltType::kNormal);

		// Manual tilt with curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& manualPoint = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse));
		REQUIRE(manualPoint.v.v == Approx(1.5));
		REQUIRE(manualPoint.curve.a == Approx(0.4));
		REQUIRE(manualPoint.curve.b == Approx(0.6));

		// Manual tilt without curve
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 2));
		const auto& manualPoint2 = std::get<kson::TiltGraphPoint>(chart.camera.tilt.at(kMeasurePulse * 2));
		REQUIRE(manualPoint2.v.v == Approx(4.0));
		REQUIRE(manualPoint2.curve.isLinear());

		// Auto tilt (bigger)
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 3));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(chart.camera.tilt.at(kMeasurePulse * 3)));
		REQUIRE(std::get<kson::AutoTiltType>(chart.camera.tilt.at(kMeasurePulse * 3)) == kson::AutoTiltType::kBigger);
	}

	SECTION("Legacy tilt format (big/keep)") {
		std::stringstream ss;
		ss << "title=Legacy Tilt Format Test\n";
		ss << "m=test.ogg\n";
		ss << "beat=4/4\n";
		ss << "t=120\n";
		ss << "o=0\n";
		ss << "--\n";
		ss << "tilt=big\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=keep\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// Legacy "big" = "bigger"
		REQUIRE(chart.camera.tilt.contains(0));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(chart.camera.tilt.at(0)));
		REQUIRE(std::get<kson::AutoTiltType>(chart.camera.tilt.at(0)) == kson::AutoTiltType::kBigger);

		// Legacy "keep" = "keep_bigger"
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(chart.camera.tilt.at(kMeasurePulse)));
		REQUIRE(std::get<kson::AutoTiltType>(chart.camera.tilt.at(kMeasurePulse)) == kson::AutoTiltType::kKeepBigger);
	}

	SECTION("Manual tilt to auto tilt immediate change") {
		std::stringstream ss;
		ss << "title=test\n";
		ss << "artist=test\n";
		ss << "effect=test\n";
		ss << "jacket=test.png\n";
		ss << "illustrator=test\n";
		ss << "difficulty=light\n";
		ss << "level=1\n";
		ss << "t=120\n";
		ss << "--\n";
		ss << "tilt=0.0\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=0.5\n";
		ss << "tilt=normal\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=0.3\n";
		ss << "0000|00|--\n";
		ss << "--\n";
		ss << "tilt=0.8\n";
		ss << "tilt=bigger\n";
		ss << "0000|00|--\n";
		ss << "--\n";

		const kson::ChartData chart = kson::LoadKSHChartData(ss);

		// First manual tilt: 0.0 at pulse 0
		REQUIRE(chart.camera.tilt.contains(0));
		const auto& tiltValue0 = chart.camera.tilt.at(0);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue0));
		REQUIRE(std::get<kson::TiltGraphPoint>(tiltValue0).v.v == Approx(0.0));

		// Manual to auto tilt: [0.5, "normal"] at pulse kMeasurePulse
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse));
		const auto& tiltValue1 = chart.camera.tilt.at(kMeasurePulse);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue1));
		const auto& point1 = std::get<kson::TiltGraphPoint>(tiltValue1);
		REQUIRE(point1.v.v == Approx(0.5));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(point1.v.vf));
		REQUIRE(std::get<kson::AutoTiltType>(point1.v.vf) == kson::AutoTiltType::kNormal);

		// Manual tilt: 0.3 at pulse kMeasurePulse * 2
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 2));
		const auto& tiltValue2 = chart.camera.tilt.at(kMeasurePulse * 2);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue2));
		REQUIRE(std::get<kson::TiltGraphPoint>(tiltValue2).v.v == Approx(0.3));

		// Manual to auto tilt: [0.8, "bigger"] at pulse kMeasurePulse * 3
		REQUIRE(chart.camera.tilt.contains(kMeasurePulse * 3));
		const auto& tiltValue3 = chart.camera.tilt.at(kMeasurePulse * 3);
		REQUIRE(std::holds_alternative<kson::TiltGraphPoint>(tiltValue3));
		const auto& point3 = std::get<kson::TiltGraphPoint>(tiltValue3);
		REQUIRE(point3.v.v == Approx(0.8));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(point3.v.vf));
		REQUIRE(std::get<kson::AutoTiltType>(point3.v.vf) == kson::AutoTiltType::kBigger);
	}
}

