#include "kson/ChartData.hpp"

namespace kson
{
	ChartData CreateEditorDefaultChartData()
	{
		ChartData chartData;

		chartData.beat.bpm.emplace(0, 120.0);
		chartData.beat.timeSig.emplace(0, TimeSig{ 4, 4 });
		chartData.beat.scrollSpeed.emplace(0, GraphValue{ 1.0, 1.0 });
		chartData.audio.bgm.vol = 0.75;
		chartData.audio.audioEffect.laser.pulseEvent.emplace("peaking_filter", std::set<Pulse>{ 0 });
		chartData.audio.audioEffect.laser.legacy.filterGain.emplace(0, 0.5);
		chartData.audio.audioEffect.laser.peakingFilterDelay = 40;
		chartData.audio.keySound.laser.vol.emplace(0, 0.5);
		chartData.camera.tilt.emplace(0, AutoTiltType::kNormal);
		chartData.bg.legacy.bg[0].filename = "desert";
		chartData.bg.legacy.bg[1].filename = "desert";
		chartData.bg.legacy.layer.filename = "arrow";

		return chartData;
	}
}
