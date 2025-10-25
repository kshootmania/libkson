#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <kson/kson.hpp>
#include <kson/io/ksh_io.hpp>
#include <kson/io/kson_io.hpp>
#include <kson/util/timing_utils.hpp>
#include <kson/util/graph_utils.hpp>
#include <kson/third_party/nlohmann/json.hpp>

#include <filesystem>
#include <sstream>

namespace {
    std::string g_assetsDir;
    std::filesystem::path g_exeDir;
}

int main(int argc, char* argv[]) {
    // Get the executable directory and construct the path to the assets directory
    std::filesystem::path exePath = argv[0];
    g_exeDir = exePath.parent_path();
    g_assetsDir = (g_exeDir / "assets").string();

    return Catch::Session().run(argc, argv);
}

TEST_CASE("Basic Chart Data", "[chart]") {
    SECTION("Empty ChartData initialization") {
        kson::ChartData chart;
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.warnings.empty());
        REQUIRE(chart.meta.title.empty());
        REQUIRE(chart.meta.titleTranslit.empty());
        REQUIRE(chart.meta.artist.empty());
        REQUIRE(chart.meta.artistTranslit.empty());
        REQUIRE(chart.meta.chartAuthor.empty());
        REQUIRE(chart.meta.difficulty.idx == 0);
        REQUIRE(chart.meta.level == 1);
        REQUIRE(chart.meta.dispBPM.empty());
    }
    
    SECTION("ChartData with metadata") {
        kson::ChartData chart;
        chart.meta.title = "Test Song";
        chart.meta.artist = "Test Artist";
        chart.meta.chartAuthor = "Test Charter";
        chart.meta.difficulty.idx = 3;
        chart.meta.level = 15;
        
        REQUIRE(chart.meta.title == "Test Song");
        REQUIRE(chart.meta.artist == "Test Artist");
        REQUIRE(chart.meta.chartAuthor == "Test Charter");
        REQUIRE(chart.meta.difficulty.idx == 3);
        REQUIRE(chart.meta.level == 15);
    }
}

TEST_CASE("Timing Utilities", "[timing]") {
    SECTION("Basic pulse conversions") {
        // 240 pulses per quarter note
        REQUIRE(kson::kResolution == 240);
        
        // 1 measure = 4 quarter notes = 960 pulses (in 4/4 time)
        kson::BeatInfo beat;
        beat.bpm[0] = 120.0;  // Add default BPM
        beat.timeSig[0] = kson::TimeSig{4, 4};
        auto cache = kson::CreateTimingCache(beat);
        
        REQUIRE(kson::MeasureIdxToPulse(1, beat, cache) == 960);
        REQUIRE(kson::MeasureIdxToPulse(2, beat, cache) == 1920);
        
        // Different time signatures
        beat.timeSig.clear();
        beat.bpm.clear();
        beat.bpm[0] = 120.0;  // Add default BPM
        beat.timeSig[0] = kson::TimeSig{3, 4};
        cache = kson::CreateTimingCache(beat);
        REQUIRE(kson::MeasureIdxToPulse(1, beat, cache) == 720);  // 3/4 time
        
        beat.timeSig.clear();
        beat.bpm.clear();
        beat.bpm[0] = 120.0;  // Add default BPM
        beat.timeSig[0] = kson::TimeSig{6, 8};
        cache = kson::CreateTimingCache(beat);
        REQUIRE(kson::MeasureIdxToPulse(1, beat, cache) == 720);  // 6/8 time
    }
    
    SECTION("BPM and time conversions") {
        kson::BeatInfo beat;
        beat.bpm.emplace(0, 120.0);  // 120 BPM at pulse 0
        beat.timeSig[0] = kson::TimeSig{4, 4};  // Default 4/4 time
        
        auto cache = kson::CreateTimingCache(beat);
        
        // At 120 BPM: 1 beat = 0.5 seconds
        // 240 pulses = 1 beat = 0.5 seconds
        REQUIRE(kson::PulseToSec(240, beat, cache) == Approx(0.5));
        REQUIRE(kson::PulseToSec(480, beat, cache) == Approx(1.0));
        
        // Test with BPM change
        beat.bpm.emplace(960, 180.0);  // Change to 180 BPM at pulse 960
        cache = kson::CreateTimingCache(beat);
        
        // First 960 pulses at 120 BPM = 2.0 seconds
        // Next 240 pulses at 180 BPM = 0.333... seconds
        REQUIRE(kson::PulseToSec(960, beat, cache) == Approx(2.0));
        REQUIRE(kson::PulseToSec(1200, beat, cache) == Approx(2.333).epsilon(0.001));
    }
    
    SECTION("Timing cache") {
        kson::BeatInfo beat;
        beat.bpm.emplace(0, 120.0);
        beat.bpm.emplace(960, 180.0);
        beat.bpm.emplace(1920, 90.0);
        beat.timeSig[0] = kson::TimeSig{4, 4};  // Default 4/4 time
        
        auto cache = kson::CreateTimingCache(beat);
        
        // Using cache should give same results
        REQUIRE(kson::PulseToSec(240, beat, cache) == Approx(0.5));
        REQUIRE(kson::PulseToSec(960, beat, cache) == Approx(2.0));
        REQUIRE(kson::PulseToSec(1200, beat, cache) == Approx(2.333).epsilon(0.001));
        
        // Reverse conversion
        REQUIRE(kson::SecToPulse(0.5, beat, cache) == 240);
        REQUIRE(kson::SecToPulse(2.0, beat, cache) == 960);
    }
}

TEST_CASE("Graph Utilities", "[graph]") {
    SECTION("Graph section value") {
        kson::Graph graph;

        // Linear interpolation
        graph.emplace(0, 0.0);
        graph.emplace(480, 1.0);
        
        REQUIRE(kson::GraphValueAt(graph, 0) == Approx(0.0));
        REQUIRE(kson::GraphValueAt(graph, 240) == Approx(0.5));
        REQUIRE(kson::GraphValueAt(graph, 480) == Approx(1.0));
        
        // After last point
        REQUIRE(kson::GraphValueAt(graph, 720) == Approx(1.0));
    }
    
}

TEST_CASE("Note Data", "[note]") {
    SECTION("BT notes") {
        kson::NoteInfo notes;
        
        // Add some BT notes
        notes.bt[0][0] = kson::Interval{480};      // BT-A at pulse 0, length 480
        notes.bt[1][240] = kson::Interval{240};    // BT-B at pulse 240, length 240
        notes.bt[2][480] = kson::Interval{0};      // BT-C at pulse 480, chip note
        notes.bt[3][720] = kson::Interval{480};    // BT-D at pulse 720, length 480
        
        REQUIRE(notes.bt[0].size() == 1);
        REQUIRE(notes.bt[1].size() == 1);
        REQUIRE(notes.bt[2].size() == 1);
        REQUIRE(notes.bt[3].size() == 1);
        
        REQUIRE(notes.bt[0].at(0).length == 480);
        REQUIRE(notes.bt[1].at(240).length == 240);
        REQUIRE(notes.bt[2].at(480).length == 0);
        REQUIRE(notes.bt[3].at(720).length == 480);
    }
    
    SECTION("FX notes") {
        kson::NoteInfo notes;
        
        // Add some FX notes
        notes.fx[0][0] = kson::Interval{480};      // FX-L at pulse 0
        notes.fx[1][960] = kson::Interval{240};    // FX-R at pulse 960
        
        REQUIRE(notes.fx[0].size() == 1);
        REQUIRE(notes.fx[1].size() == 1);
        
        REQUIRE(notes.fx[0].at(0).length == 480);
        REQUIRE(notes.fx[1].at(960).length == 240);
    }
    
    SECTION("Laser notes") {
        kson::NoteInfo notes;
        
        // Add laser sections
        notes.laser[0][0] = kson::LaserSection();
        auto& leftLaser = notes.laser[0].at(0);
        leftLaser.v.emplace(0, 0.0);
        leftLaser.v.emplace(480, 1.0);
        
        REQUIRE(notes.laser[0].size() == 1);
        REQUIRE(notes.laser[0].at(0).v.size() == 2);
    }
}

TEST_CASE("KSON Loading", "[kson_io]") {
    SECTION("Load valid KSON from string") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "title": "Test Song",
                "artist": "Test Artist",
                "chart_author": "Test Charter",
                "level": 12,
                "disp_bpm": "120",
                "std_bpm": 120.0
            },
            "beat": {
                "bpm": [[0, 120.0]],
                "time_sig": [[0, [4, 4]]]
            },
            "note": {
                "bt": [
                    [[0, 480], [960, 0]],
                    [[240, 240]],
                    [],
                    []
                ],
                "fx": [
                    [[480, 480]],
                    []
                ],
                "laser": [
                    [[0, [[0, 0.0], [480, 1.0]], 1]],
                    []
                ]
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.warnings.empty());
        
        // Check metadata
        REQUIRE(chart.meta.title == "Test Song");
        REQUIRE(chart.meta.artist == "Test Artist");
        REQUIRE(chart.meta.chartAuthor == "Test Charter");
        REQUIRE(chart.meta.level == 12);
        REQUIRE(chart.meta.dispBPM == "120");
        REQUIRE(chart.meta.stdBPM == Approx(120.0));
        
        // Check beat info
        REQUIRE(chart.beat.bpm.size() == 1);
        REQUIRE(chart.beat.bpm.at(0) == Approx(120.0));
        REQUIRE(chart.beat.timeSig.size() == 1);
        REQUIRE(chart.beat.timeSig.at(0).n == 4);
        REQUIRE(chart.beat.timeSig.at(0).d == 4);
        
        // Check notes
        REQUIRE(chart.note.bt[0].size() == 2);
        REQUIRE(chart.note.bt[0].at(0).length == 480);
        REQUIRE(chart.note.bt[0].at(960).length == 0);
        REQUIRE(chart.note.bt[1].size() == 1);
        REQUIRE(chart.note.bt[1].at(240).length == 240);
        
        REQUIRE(chart.note.fx[0].size() == 1);
        REQUIRE(chart.note.fx[0].at(480).length == 480);
        
        REQUIRE(chart.note.laser[0].size() == 1);
        REQUIRE(chart.note.laser[0].at(0).v.size() == 2);
        REQUIRE(chart.note.laser[0].at(0).v.at(0).v.v == Approx(0.0));
        REQUIRE(chart.note.laser[0].at(0).v.at(480).v.v == Approx(1.0));
        REQUIRE(chart.note.laser[0].at(0).w == kson::kLaserXScale1x);
    }
    
    SECTION("Load KSON with minimal data") {
        std::string ksonData = R"({
            "format_version": 1
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.warnings.empty());
        
        // Check defaults
        REQUIRE(chart.meta.title.empty());
        REQUIRE(chart.meta.level == 1);
        REQUIRE(chart.beat.bpm.empty());
        REQUIRE(chart.note.bt[0].empty());
    }
    
    SECTION("Load KSON with difficulty string") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "difficulty": "Maximum"
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.meta.difficulty.name == "Maximum");
    }
    
    SECTION("Load KSON with difficulty index") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "difficulty": 3
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.meta.difficulty.idx == 3);
    }
    
    SECTION("Invalid JSON") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "title": "Unclosed
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::KSONParseError);
        REQUIRE(!chart.warnings.empty());
    }
    
    SECTION("Type error in JSON") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "level": "not a number"
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::KSONParseError);
        REQUIRE(!chart.warnings.empty());
    }
    
    SECTION("Load from file path") {
        // Create a temporary test file
        std::string testFile = "test_kson.kson";
        {
            std::ofstream ofs(testFile);
            ofs << R"({
                "format_version": 1,
                "meta": {
                    "title": "File Test"
                }
            })";
        }
        
        auto chart = kson::LoadKSONChartData(testFile);
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.meta.title == "File Test");
        
        // Clean up
        std::remove(testFile.c_str());
    }
    
    SECTION("Non-existent file") {
        auto chart = kson::LoadKSONChartData("non_existent_file.kson");
        REQUIRE(chart.error == kson::ErrorType::CouldNotOpenInputFileStream);
    }
}

TEST_CASE("KSON Round-trip", "[kson_io]") {
    SECTION("Save and load KSON") {
        // Create chart data
        kson::ChartData original;
        original.meta.title = "Round Trip Test";
        original.meta.artist = "Test Artist";
        original.meta.chartAuthor = "Test Charter";
        original.meta.level = 15;
        original.meta.difficulty.idx = 3;
        original.meta.dispBPM = "120-180";
        original.meta.stdBPM = 150.0;
        
        original.beat.bpm[0] = 120.0;
        original.beat.bpm[960] = 180.0;
        original.beat.timeSig[0] = kson::TimeSig{4, 4};
        
        original.note.bt[0][0] = kson::Interval{480};
        original.note.bt[1][240] = kson::Interval{0};
        original.note.fx[0][480] = kson::Interval{240};
        
        original.note.laser[0][0] = kson::LaserSection();
        original.note.laser[0][0].v[0] = kson::GraphPoint(kson::GraphValue{0.5});
        original.note.laser[0][0].v[240] = kson::GraphPoint(kson::GraphValue{1.0});
        original.note.laser[0][0].w = kson::kLaserXScale2x;
        
        // Save to string
        std::ostringstream oss;
        auto saveResult = kson::SaveKSONChartData(oss, original);
        REQUIRE(saveResult == kson::ErrorType::None);
        
        std::string ksonString = oss.str();
        REQUIRE(!ksonString.empty());
        
        // Load back
        std::istringstream iss(ksonString);
        auto loaded = kson::LoadKSONChartData(iss);
        
        REQUIRE(loaded.error == kson::ErrorType::None);
        REQUIRE(loaded.warnings.empty());
        
        // Compare metadata
        REQUIRE(loaded.meta.title == original.meta.title);
        REQUIRE(loaded.meta.artist == original.meta.artist);
        REQUIRE(loaded.meta.chartAuthor == original.meta.chartAuthor);
        REQUIRE(loaded.meta.level == original.meta.level);
        REQUIRE(loaded.meta.difficulty.idx == original.meta.difficulty.idx);
        REQUIRE(loaded.meta.dispBPM == original.meta.dispBPM);
        REQUIRE(loaded.meta.stdBPM == Approx(original.meta.stdBPM));
        
        // Compare beat info
        REQUIRE(loaded.beat.bpm.size() == original.beat.bpm.size());
        REQUIRE(loaded.beat.bpm.at(0) == Approx(original.beat.bpm.at(0)));
        REQUIRE(loaded.beat.bpm.at(960) == Approx(original.beat.bpm.at(960)));
        REQUIRE(loaded.beat.timeSig.at(0).n == original.beat.timeSig.at(0).n);
        REQUIRE(loaded.beat.timeSig.at(0).d == original.beat.timeSig.at(0).d);
        
        // Compare notes
        REQUIRE(loaded.note.bt[0].size() == original.note.bt[0].size());
        REQUIRE(loaded.note.bt[0].at(0).length == original.note.bt[0].at(0).length);
        REQUIRE(loaded.note.bt[1].at(240).length == original.note.bt[1].at(240).length);
        REQUIRE(loaded.note.fx[0].at(480).length == original.note.fx[0].at(480).length);
        
        // Compare laser
        REQUIRE(loaded.note.laser[0].size() == original.note.laser[0].size());
        REQUIRE(loaded.note.laser[0].at(0).v.size() == original.note.laser[0].at(0).v.size());
        REQUIRE(loaded.note.laser[0].at(0).v.at(0).v.v == Approx(original.note.laser[0].at(0).v.at(0).v.v));
        REQUIRE(loaded.note.laser[0].at(0).v.at(240).v.v == Approx(original.note.laser[0].at(0).v.at(240).v.v));
        REQUIRE(loaded.note.laser[0].at(0).w == original.note.laser[0].at(0).w);
    }
}

TEST_CASE("KSON Audio Effect Loading", "[kson_io][audio_effect]") {
    SECTION("Load audio effects") {
        std::string ksonData = R"({
            "format_version": 1,
            "audio": {
                "audio_effect": {
                    "fx": {
                        "def": [
                            ["retrigger", {
                                "type": "retrigger",
                                "v": {
                                    "wave_length": "100ms",
                                    "update_period": "1/2"
                                }
                            }],
                            ["my_flanger", {
                                "type": "flanger",
                                "v": {
                                    "delay": "80samples",
                                    "depth": "30samples>40samples-60samples"
                                }
                            }]
                        ],
                        "param_change": {
                            "retrigger": {
                                "update_period": [[960, "0"], [1920, "1/2"]],
                                "update_trigger": [[1200, "on"]]
                            }
                        },
                        "long_event": {
                            "retrigger": [
                                [[480, {"wave_length": "50ms"}], [720, {"wave_length": "200ms"}]],
                                []
                            ]
                        }
                    },
                    "laser": {
                        "def": [
                            ["hpf", {
                                "type": "high_pass_filter",
                                "v": {
                                    "freq": "1kHz>2kHz"
                                }
                            }]
                        ],
                        "param_change": {
                            "hpf": {
                                "freq": [[0, "500Hz"], [960, "1kHz"]]
                            }
                        },
                        "pulse_event": {
                            "hpf": [240, 480, 720]
                        },
                        "peaking_filter_delay": 80
                    }
                }
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        
        // Check FX audio effects
        const auto& fxEffects = chart.audio.audioEffect.fx;
        REQUIRE(fxEffects.def.size() == 2);
        
        REQUIRE(fxEffects.def[0].name == "retrigger");
        REQUIRE(fxEffects.def[0].v.type == kson::AudioEffectType::Retrigger);
        REQUIRE(fxEffects.def[0].v.v.at("wave_length") == "100ms");
        REQUIRE(fxEffects.def[0].v.v.at("update_period") == "1/2");
        
        REQUIRE(fxEffects.def[1].name == "my_flanger");
        REQUIRE(fxEffects.def[1].v.type == kson::AudioEffectType::Flanger);
        REQUIRE(fxEffects.def[1].v.v.at("delay") == "80samples");
        REQUIRE(fxEffects.def[1].v.v.at("depth") == "30samples>40samples-60samples");
        
        // Check param changes
        REQUIRE(fxEffects.paramChange.count("retrigger") == 1);
        const auto& retriggerParams = fxEffects.paramChange.at("retrigger");
        REQUIRE(retriggerParams.count("update_period") == 1);
        REQUIRE(retriggerParams.at("update_period").size() == 2);
        REQUIRE(retriggerParams.at("update_period").at(960) == "0");
        REQUIRE(retriggerParams.at("update_period").at(1920) == "1/2");
        
        // Check long events
        REQUIRE(fxEffects.longEvent.count("retrigger") == 1);
        const auto& retriggerLong = fxEffects.longEvent.at("retrigger");
        REQUIRE(retriggerLong[0].size() == 2);
        REQUIRE(retriggerLong[0].at(480).at("wave_length") == "50ms");
        REQUIRE(retriggerLong[0].at(720).at("wave_length") == "200ms");
        
        // Check laser audio effects
        const auto& laserEffects = chart.audio.audioEffect.laser;
        REQUIRE(laserEffects.def.size() == 1);
        
        REQUIRE(laserEffects.def[0].name == "hpf");
        REQUIRE(laserEffects.def[0].v.type == kson::AudioEffectType::HighPassFilter);
        REQUIRE(laserEffects.def[0].v.v.at("freq") == "1kHz>2kHz");
        
        // Check laser param changes
        REQUIRE(laserEffects.paramChange.count("hpf") == 1);
        const auto& hpfParams = laserEffects.paramChange.at("hpf");
        REQUIRE(hpfParams.at("freq").size() == 2);
        REQUIRE(hpfParams.at("freq").at(0) == "500Hz");
        REQUIRE(hpfParams.at("freq").at(960) == "1kHz");
        
        // Check pulse events
        REQUIRE(laserEffects.pulseEvent.count("hpf") == 1);
        const auto& hpfPulses = laserEffects.pulseEvent.at("hpf");
        REQUIRE(hpfPulses.size() == 3);
        REQUIRE(hpfPulses.count(240) == 1);
        REQUIRE(hpfPulses.count(480) == 1);
        REQUIRE(hpfPulses.count(720) == 1);
        
        REQUIRE(laserEffects.peakingFilterDelay == 80);
    }
    
    SECTION("Load key sounds") {
        std::string ksonData = R"({
            "format_version": 1,
            "audio": {
                "key_sound": {
                    "fx": {
                        "chip_event": {
                            "clap": [
                                [[0, {"vol": 0.8}], [240, {"vol": 1.0}]],
                                [[480, {"vol": 0.5}]]
                            ],
                            "custom.wav": [
                                [[960, {"vol": 0.9}]],
                                []
                            ]
                        }
                    },
                    "laser": {
                        "vol": [[0, 0.5], [960, 1.0], [1920, 0.75]],
                        "slam_event": {
                            "slam_up": [240, 480, 720],
                            "slam_down": [960, 1200]
                        },
                        "legacy": {
                            "vol_auto": true
                        }
                    }
                }
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        
        // Check FX key sounds
        const auto& fxKeySound = chart.audio.keySound.fx;
        REQUIRE(fxKeySound.chipEvent.count("clap") == 1);
        REQUIRE(fxKeySound.chipEvent.count("custom.wav") == 1);
        
        const auto& clapLanes = fxKeySound.chipEvent.at("clap");
        REQUIRE(clapLanes[0].size() == 2);
        REQUIRE(clapLanes[0].at(0).vol == Approx(0.8));
        REQUIRE(clapLanes[0].at(240).vol == Approx(1.0));
        REQUIRE(clapLanes[1].size() == 1);
        REQUIRE(clapLanes[1].at(480).vol == Approx(0.5));
        
        // Check laser key sounds
        const auto& laserKeySound = chart.audio.keySound.laser;
        REQUIRE(laserKeySound.vol.size() == 3);
        REQUIRE(laserKeySound.vol.at(0) == Approx(0.5));
        REQUIRE(laserKeySound.vol.at(960) == Approx(1.0));
        REQUIRE(laserKeySound.vol.at(1920) == Approx(0.75));
        
        REQUIRE(laserKeySound.slamEvent.count("slam_up") == 1);
        REQUIRE(laserKeySound.slamEvent.at("slam_up").size() == 3);
        REQUIRE(laserKeySound.slamEvent.at("slam_up").count(240) == 1);
        
        REQUIRE(laserKeySound.legacy.volAuto == true);
    }
    
    SECTION("All audio effect types") {
        std::string ksonData = R"({
            "format_version": 1,
            "audio": {
                "audio_effect": {
                    "fx": {
                        "def": [
                            ["e1", {"type": "retrigger"}],
                            ["e2", {"type": "gate"}],
                            ["e3", {"type": "flanger"}],
                            ["e4", {"type": "pitch_shift"}],
                            ["e5", {"type": "bitcrusher"}],
                            ["e6", {"type": "phaser"}],
                            ["e7", {"type": "wobble"}],
                            ["e8", {"type": "tapestop"}],
                            ["e9", {"type": "echo"}],
                            ["e10", {"type": "sidechain"}],
                            ["e11", {"type": "switch_audio"}],
                            ["e12", {"type": "high_pass_filter"}],
                            ["e13", {"type": "low_pass_filter"}],
                            ["e14", {"type": "peaking_filter"}],
                            ["e15", {"type": "unknown_type"}]
                        ]
                    }
                }
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        
        const auto& defs = chart.audio.audioEffect.fx.def;
        REQUIRE(defs.size() == 15);
        
        REQUIRE(defs[0].v.type == kson::AudioEffectType::Retrigger);
        REQUIRE(defs[1].v.type == kson::AudioEffectType::Gate);
        REQUIRE(defs[2].v.type == kson::AudioEffectType::Flanger);
        REQUIRE(defs[3].v.type == kson::AudioEffectType::PitchShift);
        REQUIRE(defs[4].v.type == kson::AudioEffectType::Bitcrusher);
        REQUIRE(defs[5].v.type == kson::AudioEffectType::Phaser);
        REQUIRE(defs[6].v.type == kson::AudioEffectType::Wobble);
        REQUIRE(defs[7].v.type == kson::AudioEffectType::Tapestop);
        REQUIRE(defs[8].v.type == kson::AudioEffectType::Echo);
        REQUIRE(defs[9].v.type == kson::AudioEffectType::Sidechain);
        REQUIRE(defs[10].v.type == kson::AudioEffectType::SwitchAudio);
        REQUIRE(defs[11].v.type == kson::AudioEffectType::HighPassFilter);
        REQUIRE(defs[12].v.type == kson::AudioEffectType::LowPassFilter);
        REQUIRE(defs[13].v.type == kson::AudioEffectType::PeakingFilter);
        REQUIRE(defs[14].v.type == kson::AudioEffectType::Unspecified); // unknown_type
    }
    
    SECTION("Audio effect parameter values as strings") {
        std::string ksonData = R"({
            "format_version": 1,
            "audio": {
                "audio_effect": {
                    "fx": {
                        "def": [
                            ["retrigger", {
                                "type": "retrigger",
                                "v": {
                                    "update_period": "1/4",
                                    "wave_length": "100ms",
                                    "rate": "70%",
                                    "mix": "0%>100%"
                                }
                            }],
                            ["flanger", {
                                "type": "flanger",
                                "v": {
                                    "delay": "80samples",
                                    "depth": "30samples>40samples-60samples"
                                }
                            }]
                        ]
                    },
                    "laser": {
                        "def": [
                            ["hpf", {
                                "type": "high_pass_filter",
                                "v": {
                                    "freq": "1.5kHz",
                                    "q": "0.707"
                                }
                            }]
                        ]
                    }
                }
            }
        })";
        
        std::istringstream stream(ksonData);
        auto chart = kson::LoadKSONChartData(stream);
        REQUIRE(chart.error == kson::ErrorType::None);
        
        // Check that parameter values are stored as strings
        const auto& retrigger = chart.audio.audioEffect.fx.def[0];
        REQUIRE(retrigger.v.v.at("update_period") == "1/4");
        REQUIRE(retrigger.v.v.at("wave_length") == "100ms");
        REQUIRE(retrigger.v.v.at("rate") == "70%");
        REQUIRE(retrigger.v.v.at("mix") == "0%>100%");
        
        const auto& flanger = chart.audio.audioEffect.fx.def[1];
        REQUIRE(flanger.v.v.at("delay") == "80samples");
        REQUIRE(flanger.v.v.at("depth") == "30samples>40samples-60samples");
        
        const auto& hpf = chart.audio.audioEffect.laser.def[0];
        REQUIRE(hpf.v.v.at("freq") == "1.5kHz");
        REQUIRE(hpf.v.v.at("q") == "0.707");
    }
}

TEST_CASE("KSON BeatInfo scroll_speed", "[kson_io][beat]") {
    SECTION("Default scroll_speed") {
        std::string ksonData = R"({
            "format_version": 1,
            "beat": {
                "bpm": [[0, 120]]
            }
        })";
        
        std::istringstream stream(ksonData);
        kson::ChartData chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.beat.scrollSpeed.size() == 1);
        REQUIRE(chart.beat.scrollSpeed.count(0) == 1);
        REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
        REQUIRE(chart.beat.scrollSpeed.at(0).v.vf == Approx(1.0));
    }
    
    SECTION("Simple scroll_speed values") {
        std::string ksonData = R"({
            "format_version": 1,
            "beat": {
                "bpm": [[0, 120]],
                "scroll_speed": [
                    [0, 1.0],
                    [960, 2.0],
                    [1920, 0.5]
                ]
            }
        })";
        
        std::istringstream stream(ksonData);
        kson::ChartData chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.beat.scrollSpeed.size() == 3);
        REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
        REQUIRE(chart.beat.scrollSpeed.at(0).v.vf == Approx(1.0));
        REQUIRE(chart.beat.scrollSpeed.at(960).v.v == Approx(2.0));
        REQUIRE(chart.beat.scrollSpeed.at(960).v.vf == Approx(2.0));
        REQUIRE(chart.beat.scrollSpeed.at(1920).v.v == Approx(0.5));
        REQUIRE(chart.beat.scrollSpeed.at(1920).v.vf == Approx(0.5));
    }
    
    SECTION("scroll_speed with GraphValue arrays") {
        std::string ksonData = R"({
            "format_version": 1,
            "beat": {
                "bpm": [[0, 120]],
                "scroll_speed": [
                    [0, [1.0, 1.5]],
                    [480, 2.0],
                    [960, [3.0, 1.0]],
                    [1440, [0.5, 0.5]]
                ]
            }
        })";
        
        std::istringstream stream(ksonData);
        kson::ChartData chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.beat.scrollSpeed.size() == 4);
        
        // First point with different v and vf
        REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
        REQUIRE(chart.beat.scrollSpeed.at(0).v.vf == Approx(1.5));

        // Second point with single value (v and vf should be the same)
        REQUIRE(chart.beat.scrollSpeed.at(480).v.v == Approx(2.0));
        REQUIRE(chart.beat.scrollSpeed.at(480).v.vf == Approx(2.0));

        // Third point with different v and vf
        REQUIRE(chart.beat.scrollSpeed.at(960).v.v == Approx(3.0));
        REQUIRE(chart.beat.scrollSpeed.at(960).v.vf == Approx(1.0));

        // Fourth point with same v and vf
        REQUIRE(chart.beat.scrollSpeed.at(1440).v.v == Approx(0.5));
        REQUIRE(chart.beat.scrollSpeed.at(1440).v.vf == Approx(0.5));
    }
    
    SECTION("Empty scroll_speed array") {
        std::string ksonData = R"({
            "format_version": 1,
            "beat": {
                "bpm": [[0, 120]],
                "scroll_speed": []
            }
        })";
        
        std::istringstream stream(ksonData);
        kson::ChartData chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.beat.scrollSpeed.empty());
    }
    
    SECTION("scroll_speed with gaps (sparse points)") {
        std::string ksonData = R"({
            "format_version": 1,
            "beat": {
                "bpm": [[0, 120]],
                "scroll_speed": [
                    [0, 1.0],
                    [2400, 3.0],
                    [4800, 0.75]
                ]
            }
        })";
        
        std::istringstream stream(ksonData);
        kson::ChartData chart = kson::LoadKSONChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.beat.scrollSpeed.size() == 3);
        
        // Check that only the specified points exist
        REQUIRE(chart.beat.scrollSpeed.count(0) == 1);
        REQUIRE(chart.beat.scrollSpeed.count(2400) == 1);
        REQUIRE(chart.beat.scrollSpeed.count(4800) == 1);
        
        // Check that intermediate points do not exist
        REQUIRE(chart.beat.scrollSpeed.count(960) == 0);
        REQUIRE(chart.beat.scrollSpeed.count(1920) == 0);
        REQUIRE(chart.beat.scrollSpeed.count(3360) == 0);
        
        // Verify values
        REQUIRE(chart.beat.scrollSpeed.at(0).v.v == Approx(1.0));
        REQUIRE(chart.beat.scrollSpeed.at(2400).v.v == Approx(3.0));
        REQUIRE(chart.beat.scrollSpeed.at(4800).v.v == Approx(0.75));
    }
}

TEST_CASE("KSON BeatInfo stop", "[kson_io][beat]") {
	SECTION("No stop") {
		std::string ksonData = R"({
			"format_version": 1,
			"beat": {
				"bpm": [[0, 120]]
			}
		})";

		std::istringstream stream(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(stream);

		REQUIRE(chart.error == kson::ErrorType::None);
		REQUIRE(chart.beat.stop.empty());
	}

	SECTION("Simple stop values") {
		std::string ksonData = R"({
			"format_version": 1,
			"beat": {
				"bpm": [[0, 120]],
				"stop": [
					[960, 480],
					[1920, 240]
				]
			}
		})";

		std::istringstream stream(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(stream);

		REQUIRE(chart.error == kson::ErrorType::None);
		REQUIRE(chart.beat.stop.size() == 2);
		REQUIRE(chart.beat.stop.at(960) == 480);
		REQUIRE(chart.beat.stop.at(1920) == 240);
	}

	SECTION("Empty stop array") {
		std::string ksonData = R"({
			"format_version": 1,
			"beat": {
				"bpm": [[0, 120]],
				"stop": []
			}
		})";

		std::istringstream stream(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(stream);

		REQUIRE(chart.error == kson::ErrorType::None);
		REQUIRE(chart.beat.stop.empty());
	}

	SECTION("Stop serialization") {
		kson::ChartData chart;
		chart.beat.bpm[0] = 120.0;
		chart.beat.scrollSpeed[0] = kson::GraphValue{1.0};
		chart.beat.stop[960] = 480;
		chart.beat.stop[2400] = 240;

		std::ostringstream stream;
		kson::SaveKSONChartData(stream, chart);

		std::string result = stream.str();
		REQUIRE(result.find("\"stop\"") != std::string::npos);
		REQUIRE(result.find("[960,480]") != std::string::npos);
		REQUIRE(result.find("[2400,240]") != std::string::npos);
	}
}

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

        // Check zoom_top at pulse 0
        REQUIRE(chart.camera.cam.body.zoomTop.contains(0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.a == Approx(0.3));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.b == Approx(0.7));

        // Check zoom_bottom at pulse 150 (1/8 measure)
        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 8));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.a == Approx(0.4));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.b == Approx(0.6));

        // Check zoom_side at pulse 300 (2/8 measure)
        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.a == Approx(0.5));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.b == Approx(0.5));

        // Check center_split at pulse 450 (3/8 measure)
        REQUIRE(chart.camera.cam.body.centerSplit.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).v.v == Approx(200.0));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.2));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.8));

        // Check scroll_speed at pulse 450 (3/8 measure) - same line as center_split
        REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.v == Approx(1.5));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.3));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.6));

        // Check tilt (manual) - it's a GraphSection, so check section at pulse 600 (4/8 = 1/2 measure)
        REQUIRE(chart.camera.tilt.manual.contains(kMeasurePulse / 2));
        const auto& tiltSection = chart.camera.tilt.manual.at(kMeasurePulse / 2);
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.9));

        // Check laser L at pulse 600 (5/8 measure) - starts at '0' (0.0), moves to 'o' (1.0)
        REQUIRE(chart.note.laser[0].contains(kMeasurePulse * 5 / 8));
        const auto& laserL = chart.note.laser[0].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserL.v.contains(0));
        REQUIRE(laserL.v.at(0).v.v == Approx(0.0)); // '0' = 0/50 = 0.0
        REQUIRE(laserL.v.at(0).curve.a == Approx(0.6));
        REQUIRE(laserL.v.at(0).curve.b == Approx(0.4));
        // Second point at relative pulse 240 (2 lines later: line 5 -> line 7)
        REQUIRE(laserL.v.contains(240));
        REQUIRE(laserL.v.at(240).v.v == Approx(1.0)); // 'o' = 50/50 = 1.0

        // Check laser R at pulse 600 (5/8 measure) - starts at 'o' (1.0), moves to '0' (0.0)
        REQUIRE(chart.note.laser[1].contains(kMeasurePulse * 5 / 8));
        const auto& laserR = chart.note.laser[1].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserR.v.contains(0));
        REQUIRE(laserR.v.at(0).v.v == Approx(1.0)); // 'o' = 50/50 = 1.0
        REQUIRE(laserR.v.at(0).curve.a == Approx(0.7));
        REQUIRE(laserR.v.at(0).curve.b == Approx(0.3));
        // Second point at relative pulse 240 (2 lines later)
        REQUIRE(laserR.v.contains(240));
        REQUIRE(laserR.v.at(240).v.v == Approx(0.0)); // '0' = 0/50 = 0.0
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

        // Check zoom_top at pulse 0 (should have curve even though curve line is after)
        REQUIRE(chart.camera.cam.body.zoomTop.contains(0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.a == Approx(0.3));
        REQUIRE(chart.camera.cam.body.zoomTop.at(0).curve.b == Approx(0.7));

        // Check zoom_bottom at pulse 150 (1/8 measure)
        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 8));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.a == Approx(0.4));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 8).curve.b == Approx(0.6));

        // Check zoom_side at pulse 300 (2/8 measure)
        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.a == Approx(0.5));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse / 4).curve.b == Approx(0.5));

        // Check center_split at pulse 450 (3/8 measure)
        REQUIRE(chart.camera.cam.body.centerSplit.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).v.v == Approx(200.0));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.2));
        REQUIRE(chart.camera.cam.body.centerSplit.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.8));

        // Check scroll_speed at pulse 450 (3/8 measure) - same line as center_split (curve after parameter)
        REQUIRE(chart.beat.scrollSpeed.contains(kMeasurePulse * 3 / 8));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).v.v == Approx(1.5));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.a == Approx(0.3));
        REQUIRE(chart.beat.scrollSpeed.at(kMeasurePulse * 3 / 8).curve.b == Approx(0.6));

        // Check tilt at pulse 600 (4/8 = 1/2 measure)
        REQUIRE(chart.camera.tilt.manual.contains(kMeasurePulse / 2));
        const auto& tiltSection = chart.camera.tilt.manual.at(kMeasurePulse / 2);
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.9));

        // Check laser L at pulse 600 (5/8 measure) - starts at '0', moves to 'o'
        REQUIRE(chart.note.laser[0].contains(kMeasurePulse * 5 / 8));
        const auto& laserL = chart.note.laser[0].at(kMeasurePulse * 5 / 8);
        REQUIRE(laserL.v.contains(0));
        REQUIRE(laserL.v.at(0).v.v == Approx(0.0));
        REQUIRE(laserL.v.at(0).curve.a == Approx(0.6));
        REQUIRE(laserL.v.at(0).curve.b == Approx(0.4));
        REQUIRE(laserL.v.contains(240));
        REQUIRE(laserL.v.at(240).v.v == Approx(1.0));

        // Check laser R at pulse 600 (5/8 measure) - starts at 'o', moves to '0'
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
        ss << "zoom_top_curve=0.3;0.7\n"; // pulse 0
        ss << "0000|00|--\n";
        ss << "zoom_top=100\n"; // pulse 240 - different from curve
        ss << "0000|00|--\n";
        ss << "zoom_bottom=50\n"; // pulse 480
        ss << "0000|00|--\n";
        ss << "zoom_bottom_curve=0.4;0.6\n"; // pulse 720 - different from zoom_bottom
        ss << "zoom_side_curve=0.5;0.5\n"; // pulse 720 - same as zoom_bottom_curve
        ss << "0000|00|--\n";
        ss << "--\n";
        ss << "zoom_side=-25\n"; // pulse 1200 (measure 1, line 0) - different measure and different pulse
        ss << "0000|00|--\n";
        ss << "center_split_curve=0.2;0.8\n"; // pulse 1440
        ss << "0000|00|--\n";
        ss << "center_split=200\n"; // pulse 1680 - different from curve
        ss << "0000|00|--\n";
        ss << "tilt_curve=0.1;0.9\n"; // pulse 1920
        ss << "laser_l_curve=0.6;0.4\n"; // pulse 1920
        ss << "0000|00|0-\n";
        ss << "0000|00|o-\n";
        ss << "tilt=0.1\n"; // pulse 2400 - different from curve
        ss << "laser_r_curve=0.7;0.3\n"; // pulse 2400
        ss << "0000|00|-0\n";
        ss << "0000|00|-o\n";
        ss << "--\n";

        kson::ChartData chart = kson::LoadKSHChartData(ss);
        REQUIRE(chart.error == kson::ErrorType::None);

        // zoom_top at pulse 240 should NOT have curve (curve was at pulse 0)
        REQUIRE(chart.camera.cam.body.zoomTop.contains(kMeasurePulse / 4));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).v.v == Approx(100.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomTop.at(kMeasurePulse / 4).curve.b == Approx(0.0));

        // zoom_bottom at pulse 480 should NOT have curve (curve was at pulse 720)
        REQUIRE(chart.camera.cam.body.zoomBottom.contains(kMeasurePulse / 2));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).v.v == Approx(50.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomBottom.at(kMeasurePulse / 2).curve.b == Approx(0.0));

        // zoom_side at pulse 1200 should NOT have curve (curve was at different pulse 720)
        REQUIRE(chart.camera.cam.body.zoomSide.contains(kMeasurePulse));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).v.v == Approx(-25.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).curve.a == Approx(0.0));
        REQUIRE(chart.camera.cam.body.zoomSide.at(kMeasurePulse).curve.b == Approx(0.0));

        // center_split should NOT have curve (curve was at different pulse)
        // Note: Pulse calculation depends on line distribution in the measure
        const auto centerSplitItr = chart.camera.cam.body.centerSplit.begin();
        REQUIRE(centerSplitItr != chart.camera.cam.body.centerSplit.end());
        REQUIRE(centerSplitItr->second.v.v == Approx(200.0));
        REQUIRE(centerSplitItr->second.curve.a == Approx(0.0));
        REQUIRE(centerSplitItr->second.curve.b == Approx(0.0));

        // tilt should NOT have curve (curve was at different pulse)
        // Note: Pulse calculation depends on line distribution in the measure
        REQUIRE(chart.camera.tilt.manual.size() == 1);
        const auto& tiltSection = chart.camera.tilt.manual.begin()->second;
        REQUIRE(tiltSection.v.contains(0));
        REQUIRE(tiltSection.v.at(0).v.v == Approx(0.1));
        REQUIRE(tiltSection.v.at(0).curve.a == Approx(0.0));
        REQUIRE(tiltSection.v.at(0).curve.b == Approx(0.0));

        // laser L should NOT have curve at second point (curve was only for first point)
        // Note: Pulse calculation depends on line distribution in the measure
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

TEST_CASE("KSON format_version validation", "[kson_io]") {
	SECTION("Missing format_version field") {
		std::string ksonData = R"({
			"meta": {
				"title": "Test",
				"artist": "Test",
				"chart_author": "Test",
				"level": 1,
				"disp_bpm": "120"
			},
			"beat": {
				"bpm": [[0, 120.0]]
			}
		})";

		std::istringstream iss(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(iss);

		REQUIRE(chart.error == kson::ErrorType::KSONParseError);
		REQUIRE(chart.warnings.size() > 0);
		REQUIRE(chart.warnings[0] == "Missing required field: format_version");
	}

	SECTION("Invalid format_version type (string)") {
		std::string ksonData = R"({
			"format_version": "1",
			"meta": {
				"title": "Test",
				"artist": "Test",
				"chart_author": "Test",
				"level": 1,
				"disp_bpm": "120"
			},
			"beat": {
				"bpm": [[0, 120.0]]
			}
		})";

		std::istringstream iss(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(iss);

		REQUIRE(chart.error == kson::ErrorType::KSONParseError);
		REQUIRE(chart.warnings.size() > 0);
		REQUIRE(chart.warnings[0] == "Invalid format_version: must be an integer");
	}

	SECTION("Valid format_version") {
		std::string ksonData = R"({
			"format_version": 1,
			"meta": {
				"title": "Test",
				"artist": "Test",
				"chart_author": "Test",
				"level": 1,
				"disp_bpm": "120"
			},
			"beat": {
				"bpm": [[0, 120.0]]
			}
		})";

		std::istringstream iss(ksonData);
		kson::ChartData chart = kson::LoadKSONChartData(iss);

		REQUIRE(chart.error == kson::ErrorType::None);
	}
}

