#pragma once
#include "kson/Common/Common.hpp"
#include "kson/Note/NoteInfo.hpp"
#include "kson/Beat/BeatInfo.hpp"
#include "kson/ChartData.hpp"

namespace kson
{
	struct TimingCache
	{
		std::map<Pulse, double> bpmChangeSec;
		std::map<double, Pulse> bpmChangePulse;
		std::map<std::int64_t, Pulse> timeSigChangePulse;
		std::map<Pulse, std::int64_t> timeSigChangeMeasureIdx;
	};

	[[nodiscard]]
	Pulse TimeSigOneMeasurePulse(const TimeSig& timeSig);

	[[nodiscard]]
	TimingCache CreateTimingCache(const BeatInfo& beatInfo);

	[[nodiscard]]
	double PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	double PulseToSec(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double PulseDoubleToMs(double pulse, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	double PulseDoubleToSec(double pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	Pulse MsToPulse(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	Pulse SecToPulse(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double MsToPulseDouble(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	double SecToPulseDouble(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	std::int64_t PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	std::int64_t MsToMeasureIdx(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	std::int64_t SecToMeasureIdx(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	Pulse MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	Pulse MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double MeasureValueToPulseDouble(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	double MeasureIdxToSec(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);
	[[nodiscard]]
	double MeasureValueToSec(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	bool IsBarLinePulse(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double TempoAt(Pulse pulse, const BeatInfo& beatInfo);

	[[nodiscard]]
	const TimeSig& TimeSigAt(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	[[nodiscard]]
	double GetModeBPM(const BeatInfo& beatInfo, Pulse lastPulse);

	[[nodiscard]]
	double GetEffectiveStdBPM(const ChartData& chartData);

	[[nodiscard]]
	kson::Pulse LastNoteEndY(const kson::NoteInfo& noteInfo);
	[[nodiscard]]
	kson::Pulse LastNoteEndYButtonLane(const kson::ByPulse<kson::Interval>& lane);
	[[nodiscard]]
	kson::Pulse LastNoteEndYLaserLane(const kson::ByPulse<kson::LaserSection>& lane);
}
