#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/IO/KshIO.hpp>
#include <kson/IO/KsonIO.hpp>
#include <kson/third_party/nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

extern std::string g_assetsDir;
extern std::filesystem::path g_exeDir;

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
        auto chart = kson::LoadKsonChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
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
        auto chart = kson::LoadKsonChartData(stream);
        
        REQUIRE(chart.error == kson::ErrorType::None);
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
        auto chart = kson::LoadKsonChartData(stream);
        
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
        auto chart = kson::LoadKsonChartData(stream);
        
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
        kson::KsonLoadingDiag ksonDiag;
        auto chart = kson::LoadKsonChartData(stream, &ksonDiag);

        REQUIRE(chart.error == kson::ErrorType::KsonParseError);
        REQUIRE(!ksonDiag.warnings.empty());
    }

    SECTION("Type error in JSON") {
        std::string ksonData = R"({
            "format_version": 1,
            "meta": {
                "level": "not a number"
            }
        })";

        std::istringstream stream(ksonData);
        kson::KsonLoadingDiag ksonDiag;
        auto chart = kson::LoadKsonChartData(stream, &ksonDiag);

        REQUIRE(chart.error == kson::ErrorType::KsonParseError);
        REQUIRE(!ksonDiag.warnings.empty());
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
        
        auto chart = kson::LoadKsonChartData(testFile);
        REQUIRE(chart.error == kson::ErrorType::None);
        REQUIRE(chart.meta.title == "File Test");
        
        // Clean up
        std::remove(testFile.c_str());
    }
    
    SECTION("Non-existent file") {
        auto chart = kson::LoadKsonChartData("non_existent_file.kson");
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
        auto saveResult = kson::SaveKsonChartData(oss, original);
        REQUIRE(saveResult == kson::ErrorType::None);
        
        std::string ksonString = oss.str();
        REQUIRE(!ksonString.empty());
        
        // Load back
        std::istringstream iss(ksonString);
        auto loaded = kson::LoadKsonChartData(iss);
        
        REQUIRE(loaded.error == kson::ErrorType::None);
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
        auto chart = kson::LoadKsonChartData(stream);
        
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
        auto chart = kson::LoadKsonChartData(stream);
        
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
        auto chart = kson::LoadKsonChartData(stream);
        
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
        auto chart = kson::LoadKsonChartData(stream);
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
        kson::ChartData chart = kson::LoadKsonChartData(stream);
        
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
        kson::ChartData chart = kson::LoadKsonChartData(stream);
        
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
        kson::ChartData chart = kson::LoadKsonChartData(stream);
        
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
        kson::ChartData chart = kson::LoadKsonChartData(stream);
        
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
        kson::ChartData chart = kson::LoadKsonChartData(stream);
        
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
		kson::ChartData chart = kson::LoadKsonChartData(stream);

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
		kson::ChartData chart = kson::LoadKsonChartData(stream);

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
		kson::ChartData chart = kson::LoadKsonChartData(stream);

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
		kson::SaveKsonChartData(stream, chart);

		std::string result = stream.str();
		REQUIRE(result.find("\"stop\"") != std::string::npos);
		REQUIRE(result.find("[960,480]") != std::string::npos);
		REQUIRE(result.find("[2400,240]") != std::string::npos);
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
		kson::KsonLoadingDiag ksonDiag;
		kson::ChartData chart = kson::LoadKsonChartData(iss, &ksonDiag);

		REQUIRE(chart.error == kson::ErrorType::KsonParseError);
		REQUIRE(ksonDiag.warnings.size() > 0);
		REQUIRE(ksonDiag.warnings[0].type == kson::KsonLoadingWarningType::MissingFormatVersion);
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
		kson::KsonLoadingDiag ksonDiag;
		kson::ChartData chart = kson::LoadKsonChartData(iss, &ksonDiag);

		REQUIRE(chart.error == kson::ErrorType::KsonParseError);
		REQUIRE(ksonDiag.warnings.size() > 0);
		REQUIRE(ksonDiag.warnings[0].type == kson::KsonLoadingWarningType::InvalidFormatVersion);
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
		kson::ChartData chart = kson::LoadKsonChartData(iss);

		REQUIRE(chart.error == kson::ErrorType::None);
	}
}

TEST_CASE("KSON Tilt Serialization", "[kson_io][tilt]") {
	SECTION("All tilt format variations") {
		// Create chart with all tilt format types
		kson::ChartData chart;
		chart.meta.title = "Tilt Test";
		chart.meta.artist = "Test";
		chart.meta.chartAuthor = "Test";
		chart.meta.level = 1;
		chart.meta.dispBPM = "120";
		chart.beat.bpm[0] = 120.0;
		chart.beat.timeSig[0] = {4, 4};

		// Format 1: [pulse, "string"]
		chart.camera.tilt[0] = kson::AutoTiltType::kNormal;
		chart.camera.tilt[480] = kson::AutoTiltType::kBigger;

		// Format 2: [pulse, double]
		chart.camera.tilt[960] = kson::TiltGraphPoint{kson::TiltGraphValue{2.5}};

		// Format 3: [pulse, [double, double]]
		chart.camera.tilt[1440] = kson::TiltGraphPoint{kson::TiltGraphValue{1.0, 3.0}};

		// Format 4: [pulse, [double, string]]
		chart.camera.tilt[1920] = kson::TiltGraphPoint{kson::TiltGraphValue{0.8, kson::AutoTiltType::kBigger}};

		// Format 5: [pulse, [v, [a, b]]]
		chart.camera.tilt[2400] = kson::TiltGraphPoint{kson::TiltGraphValue{1.5}, kson::GraphCurveValue{0.2, 0.8}};

		// Format 6: [pulse, [[double, double], [a, b]]]
		chart.camera.tilt[2880] = kson::TiltGraphPoint{kson::TiltGraphValue{0.0, 5.0}, kson::GraphCurveValue{0.3, 0.7}};

		// Save to KSON
		std::ostringstream oss;
		kson::ErrorType result = kson::SaveKsonChartData(oss, chart);
		REQUIRE(result == kson::ErrorType::None);

		std::string ksonOutput = oss.str();
		REQUIRE(!ksonOutput.empty());

		// Verify JSON structure
		nlohmann::json j = nlohmann::json::parse(ksonOutput);
		REQUIRE(j.contains("camera"));
		REQUIRE(j["camera"].contains("tilt"));
		REQUIRE(j["camera"]["tilt"].is_array());

		auto& tiltArray = j["camera"]["tilt"];
		REQUIRE(tiltArray.size() == 7);

		// Verify auto tilt (normal)
		REQUIRE(tiltArray[0][0] == 0);
		REQUIRE(tiltArray[0][1] == "normal");

		// Verify auto tilt (bigger)
		REQUIRE(tiltArray[1][0] == 480);
		REQUIRE(tiltArray[1][1] == "bigger");

		// Verify simple value [pulse, double]
		REQUIRE(tiltArray[2][0] == 960);
		REQUIRE(tiltArray[2][1] == 2.5);

		// Verify immediate change [pulse, [double, double]]
		REQUIRE(tiltArray[3][0] == 1440);
		REQUIRE(tiltArray[3][1].is_array());
		REQUIRE(tiltArray[3][1].size() == 2);
		REQUIRE(tiltArray[3][1][0] == 1.0);
		REQUIRE(tiltArray[3][1][1] == 3.0);

		// Verify manual to auto tilt [pulse, [double, string]]
		REQUIRE(tiltArray[4][0] == 1920);
		REQUIRE(tiltArray[4][1].is_array());
		REQUIRE(tiltArray[4][1].size() == 2);
		REQUIRE(tiltArray[4][1][0] == 0.8);
		REQUIRE(tiltArray[4][1][1] == "bigger");

		// Verify curve only [pulse, [v, [a, b]]]
		REQUIRE(tiltArray[5][0] == 2400);
		REQUIRE(tiltArray[5][1].is_array());
		REQUIRE(tiltArray[5][1].size() == 2);
		REQUIRE(tiltArray[5][1][0] == 1.5);
		REQUIRE(tiltArray[5][1][1].is_array());
		REQUIRE(tiltArray[5][1][1][0] == 0.2);
		REQUIRE(tiltArray[5][1][1][1] == 0.8);

		// Verify immediate change with curve [pulse, [[double, double], [a, b]]]
		REQUIRE(tiltArray[6][0] == 2880);
		REQUIRE(tiltArray[6][1].is_array());
		REQUIRE(tiltArray[6][1].size() == 2);
		REQUIRE(tiltArray[6][1][0].is_array());
		REQUIRE(tiltArray[6][1][0][0] == 0.0);
		REQUIRE(tiltArray[6][1][0][1] == 5.0);
		REQUIRE(tiltArray[6][1][1].is_array());
		REQUIRE(tiltArray[6][1][1][0] == 0.3);
		REQUIRE(tiltArray[6][1][1][1] == 0.7);

		// Load back
		std::istringstream iss(ksonOutput);
		kson::ChartData loaded = kson::LoadKsonChartData(iss);
		REQUIRE(loaded.error == kson::ErrorType::None);

		// Compare tilt data
		REQUIRE(loaded.camera.tilt.size() == 7);

		REQUIRE(std::holds_alternative<kson::AutoTiltType>(loaded.camera.tilt.at(0)));
		REQUIRE(std::get<kson::AutoTiltType>(loaded.camera.tilt.at(0)) == kson::AutoTiltType::kNormal);

		REQUIRE(std::holds_alternative<kson::AutoTiltType>(loaded.camera.tilt.at(480)));
		REQUIRE(std::get<kson::AutoTiltType>(loaded.camera.tilt.at(480)) == kson::AutoTiltType::kBigger);

		const auto& point960 = std::get<kson::TiltGraphPoint>(loaded.camera.tilt.at(960));
		REQUIRE(point960.v.v == Approx(2.5));
		REQUIRE(std::holds_alternative<double>(point960.v.vf));
		REQUIRE(std::get<double>(point960.v.vf) == Approx(2.5));
		REQUIRE(point960.curve.isLinear());

		const auto& point1440 = std::get<kson::TiltGraphPoint>(loaded.camera.tilt.at(1440));
		REQUIRE(point1440.v.v == Approx(1.0));
		REQUIRE(std::holds_alternative<double>(point1440.v.vf));
		REQUIRE(std::get<double>(point1440.v.vf) == Approx(3.0));
		REQUIRE(point1440.curve.isLinear());

		const auto& point1920 = std::get<kson::TiltGraphPoint>(loaded.camera.tilt.at(1920));
		REQUIRE(point1920.v.v == Approx(0.8));
		REQUIRE(std::holds_alternative<kson::AutoTiltType>(point1920.v.vf));
		REQUIRE(std::get<kson::AutoTiltType>(point1920.v.vf) == kson::AutoTiltType::kBigger);
		REQUIRE(point1920.curve.isLinear());

		const auto& point2400 = std::get<kson::TiltGraphPoint>(loaded.camera.tilt.at(2400));
		REQUIRE(point2400.v.v == Approx(1.5));
		REQUIRE(std::holds_alternative<double>(point2400.v.vf));
		REQUIRE(std::get<double>(point2400.v.vf) == Approx(1.5));
		REQUIRE(point2400.curve.a == Approx(0.2));
		REQUIRE(point2400.curve.b == Approx(0.8));

		const auto& point2880 = std::get<kson::TiltGraphPoint>(loaded.camera.tilt.at(2880));
		REQUIRE(point2880.v.v == Approx(0.0));
		REQUIRE(std::holds_alternative<double>(point2880.v.vf));
		REQUIRE(std::get<double>(point2880.v.vf) == Approx(5.0));
		REQUIRE(point2880.curve.a == Approx(0.3));
		REQUIRE(point2880.curve.b == Approx(0.7));
	}
}
