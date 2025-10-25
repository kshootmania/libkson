#include <catch2/catch.hpp>
#include <kson/kson.hpp>
#include <kson/util/timing_utils.hpp>
#include <kson/util/graph_utils.hpp>

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
