#pragma once
#include "kson/common/common.hpp"
#include "kson/note/note_info.hpp"
#include "kson/beat/beat_info.hpp"
#include "kson/chart_data.hpp"

namespace kson
{
	struct TimingCache
	{
		std::map<Pulse, double> bpmChangeSec;
		std::map<double, Pulse> bpmChangePulse;
		std::map<std::int64_t, Pulse> timeSigChangePulse;
		std::map<Pulse, std::int64_t> timeSigChangeMeasureIdx;
	};

	Pulse TimeSigOneMeasurePulse(const TimeSig& timeSig);

	TimingCache CreateTimingCache(const BeatInfo& beatInfo);

	double PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);
	double PulseToSec(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	double PulseDoubleToMs(double pulse, const BeatInfo& beatInfo, const TimingCache& cache);
	double PulseDoubleToSec(double pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MsToPulse(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	Pulse SecToPulse(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	double MsToPulseDouble(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	double SecToPulseDouble(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	std::int64_t PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	std::int64_t MsToMeasureIdx(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	std::int64_t SecToMeasureIdx(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	double MeasureValueToPulseDouble(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	double MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);
	double MeasureIdxToSec(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	double MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);
	double MeasureValueToSec(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	bool IsBarLinePulse(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	double TempoAt(Pulse pulse, const BeatInfo& beatInfo);

	const TimeSig& TimeSigAt(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	double GetModeBPM(const BeatInfo& beatInfo);

	double GetEffectiveStdBPM(const ChartData& chartData);

	kson::Pulse LastNoteEndY(const kson::NoteInfo& noteInfo);
	kson::Pulse LastNoteEndYButtonLane(const kson::ByPulse<kson::Interval>& lane);
	kson::Pulse LastNoteEndYLaserLane(const kson::ByPulse<kson::LaserSection>& lane);
}
