#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/io/ksh_io.hpp>
#include <kson/io/kson_io.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

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

TEST_CASE("KSON I/O round-trip (all songs)", "[ksh_io][kson_io][round_trip][all_songs]") {
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

