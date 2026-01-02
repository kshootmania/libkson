#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/Util/TimingUtils.hpp>
#include <kson/Util/GraphUtils.hpp>

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

	SECTION("GetModeBPM") {
		kson::BeatInfo beat;

		// Single BPM
		beat.bpm.clear();
		beat.bpm.emplace(0, 120.0);
		beat.timeSig[0] = kson::TimeSig{4, 4};
		REQUIRE(kson::GetModeBPM(beat, 1920) == Approx(120.0));

		// Multiple BPMs with different durations
		beat.bpm.clear();
		beat.bpm.emplace(0, 120.0);
		beat.bpm.emplace(480, 180.0);
		beat.bpm.emplace(960, 120.0);
		// 120: 0-480 + 960-1920 = 1440 pulses, 180: 480-960 = 480 pulses
		REQUIRE(kson::GetModeBPM(beat, 1920) == Approx(120.0));

		// Tied durations - higher BPM preferred
		beat.bpm.clear();
		beat.bpm.emplace(0, 120.0);
		beat.bpm.emplace(480, 180.0);
		beat.bpm.emplace(1440, 150.0);
		// 180 and 150 both have 960 pulses, higher BPM (180) wins
		REQUIRE(kson::GetModeBPM(beat, 2400) == Approx(180.0));

		// Repeated BPM values accumulate
		beat.bpm.clear();
		beat.bpm.emplace(0, 200.0);
		beat.bpm.emplace(240, 150.0);
		beat.bpm.emplace(720, 200.0);
		beat.bpm.emplace(1200, 180.0);
		beat.bpm.emplace(2160, 150.0);
		// 150: 480 + 840 = 1320, 180: 960, 200: 720
		REQUIRE(kson::GetModeBPM(beat, 3000) == Approx(150.0));

		// Decimal values are distinguished up to 3 decimal places
		beat.bpm.clear();
		beat.bpm.emplace(0, 150.2);
		beat.bpm.emplace(480, 150.7);
		beat.bpm.emplace(960, 180.0);
		// 150.2 and 150.7 each have 480 pulses, higher BPM (150.7) wins
		REQUIRE(kson::GetModeBPM(beat, 1200) == Approx(150.7));

		// 4th decimal place is ignored (truncated to 3 decimal places)
		beat.bpm.clear();
		beat.bpm.emplace(0, 150.1231);
		beat.bpm.emplace(480, 150.1239);
		beat.bpm.emplace(960, 120.0);
		// 150.1231 and 150.1239 both become 150.123: 960 pulses, 120.0: 240 pulses
		REQUIRE(kson::GetModeBPM(beat, 1200) == Approx(150.123).epsilon(0.0001));

		// BPM changes after lastPulse are ignored
		beat.bpm.clear();
		beat.bpm.emplace(0, 120.0);
		beat.bpm.emplace(480, 180.0);
		beat.bpm.emplace(1200, 150.0);
		// Both 120 and 180 have 480 pulses, higher BPM (180) wins
		REQUIRE(kson::GetModeBPM(beat, 960) == Approx(180.0));
	}

	SECTION("GetEffectiveStdBPM") {
		kson::ChartData chart;

		// Explicit stdBPM takes precedence
		chart.meta.stdBPM = 175.0;
		chart.beat.bpm.clear();
		chart.beat.bpm.emplace(0, 120.0);
		chart.beat.bpm.emplace(480, 180.0);
		chart.beat.bpm.emplace(960, 120.0);
		chart.beat.timeSig[0] = kson::TimeSig{4, 4};
		chart.note.bt[0][0] = kson::Interval{480};
		chart.note.bt[1][960] = kson::Interval{480};
		REQUIRE(kson::GetEffectiveStdBPM(chart) == Approx(175.0));

		// Without explicit stdBPM, calculate mode BPM up to last note
		chart.meta.stdBPM = 0.0;
		// Last note ends at 960+480=1440, so 120 has 960 pulses, 180 has 480 pulses
		REQUIRE(kson::GetEffectiveStdBPM(chart) == Approx(120.0));

		// Negative stdBPM treated as unset
		chart.meta.stdBPM = -1.0;
		REQUIRE(kson::GetEffectiveStdBPM(chart) == Approx(120.0));
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

	SECTION("Graph with linear curve control point") {
		for (const double value : {0.0, 0.25, 0.5, 0.75, 1.0})
		{
			kson::Graph graph;
			
			graph.emplace(0, kson::GraphPoint{0.0, {value, value}});
			graph.emplace(480, 1.0);

			REQUIRE(kson::GraphValueAt(graph, 240) == Approx(0.5));
		}
	}

	SECTION("Graph with common curve control points") {
		// Strong ease-in curve
		{
			kson::Graph graph;
			
			graph.emplace(0, kson::GraphPoint{0.0, {1.0, 0.0}});
			graph.emplace(480, 1.0);

			REQUIRE(kson::GraphValueAt(graph, 120) == Approx(1.75 - std::sqrt(3.0)));
			REQUIRE(kson::GraphValueAt(graph, 240) == Approx(1.50 - std::sqrt(2.0)));
			REQUIRE(kson::GraphValueAt(graph, 360) == Approx(0.25));
		}

		// Strong ease-out curve
		{
			kson::Graph graph;
			
			graph.emplace(0, kson::GraphPoint{0.0, {0.0, 1.0}});
			graph.emplace(480, 1.0);

			REQUIRE(kson::GraphValueAt(graph, 120) == Approx(0.75));
			REQUIRE(kson::GraphValueAt(graph, 240) == Approx(std::sqrt(2.0) - 0.50));
			REQUIRE(kson::GraphValueAt(graph, 360) == Approx(std::sqrt(3.0) - 0.75));
		}

		// Weak ease-out curve, flat end
		{
			kson::Graph graph;
			
			graph.emplace(0, kson::GraphPoint{0.0, {0.5, 1.0}});
			graph.emplace(480, 1.0);

			REQUIRE(kson::GraphValueAt(graph, 120) == Approx(0.4375));
			REQUIRE(kson::GraphValueAt(graph, 240) == Approx(0.7500));
			REQUIRE(kson::GraphValueAt(graph, 360) == Approx(0.9375));
		}
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
