#pragma once
#include "kson/common/common.hpp"
#include "kson/beat/beat_info.hpp"

namespace kson
{
	struct TimingCache
	{
		std::map<Pulse, double> bpmChangeSec;
		std::map<double, Pulse> bpmChangePulse;
		std::map<std::int64_t, Pulse> timeSigChangePulse;
		std::map<Pulse, std::int64_t> timeSigChangeMeasureIdx;
	};

	Pulse TimeSigMeasurePulse(const TimeSig& timeSig);

	TimingCache CreateTimingCache(const BeatInfo& beatInfo);

	double PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);
	double PulseToSec(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MsToPulse(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	Pulse SecToPulse(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	std::int64_t PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	std::int64_t MsToMeasureIdx(double ms, const BeatInfo& beatInfo, const TimingCache& cache);
	std::int64_t SecToMeasureIdx(double sec, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	Pulse MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	double MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);
	double MeasureIdxToSec(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache);

	double MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);
	double MeasureValueToSec(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache);

	bool IsPulseBarLine(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);

	double PulseTempo(Pulse pulse, const BeatInfo& beatInfo);

	const TimeSig& PulseTimeSig(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache);
}
