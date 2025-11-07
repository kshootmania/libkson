#include "kson/Util/TimingUtils.hpp"
#include <optional>
#include <cassert>

kson::Pulse kson::TimeSigOneMeasurePulse(const TimeSig& timeSig)
{
	return kResolution4 * static_cast<Pulse>(timeSig.n) / static_cast<Pulse>(timeSig.d);
}

kson::TimingCache kson::CreateTimingCache(const BeatInfo& beatInfo)
{
	// There must be at least one tempo change
	assert(beatInfo.bpm.size() > 0);

	// Tempo at zero position must be set
	assert(beatInfo.bpm.contains(0));

	// There must be at least one time signature change
	assert(beatInfo.timeSig.size() > 0);

	// Time signature at zero position must be set
	assert(beatInfo.timeSig.contains(0));

	TimingCache cache;

	// The first tempo change should be placed at 0.0s
	cache.bpmChangeSec.emplace(0, 0.0);
	cache.bpmChangePulse.emplace(0.0, 0);

	// The first time signature change should be placed at zero
	cache.timeSigChangePulse.emplace(0, 0);
	cache.timeSigChangeMeasureIdx.emplace(0, 0);

	// Calculate sec for each tempo change
	{
		double sec = 0.0;
		auto prevItr = beatInfo.bpm.cbegin();
		for (auto itr = std::next(prevItr); itr != beatInfo.bpm.cend(); ++itr)
		{
			sec += static_cast<double>(itr->first - prevItr->first) / kResolution * 60 / prevItr->second;
			cache.bpmChangeSec[itr->first] = sec;
			cache.bpmChangePulse[sec] = itr->first;
			prevItr = itr;
		}
	}

	// Calculate measure count for each time signature change
	{
		Pulse pulse = 0;
		auto prevItr = beatInfo.timeSig.cbegin();
		for (auto itr = std::next(prevItr); itr != beatInfo.timeSig.cend(); ++itr)
		{
			pulse += (itr->first - prevItr->first) * TimeSigOneMeasurePulse(prevItr->second);
			cache.timeSigChangePulse[itr->first] = pulse;
			cache.timeSigChangeMeasureIdx[pulse] = itr->first;
			prevItr = itr;
		}
	}

	return cache;
}

double kson::PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return PulseToSec(pulse, beatInfo, cache) * 1000;
}

double kson::PulseToSec(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest BPM change
	assert(!beatInfo.bpm.empty());
	const auto itr = ValueItrAt(beatInfo.bpm, pulse);
	const Pulse nearestBPMChangePulse = itr->first;
	const double nearestBPM = itr->second;

	// Calculate sec using pulse difference from nearest tempo change
	return cache.bpmChangeSec.at(nearestBPMChangePulse) + static_cast<double>(pulse - nearestBPMChangePulse) / kResolution * 60 / nearestBPM;
}

double kson::PulseDoubleToMs(double pulseDouble, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return PulseDoubleToSec(pulseDouble, beatInfo, cache) * 1000;
}

double kson::PulseDoubleToSec(double pulseDouble, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest BPM change
	assert(!beatInfo.bpm.empty());
	const auto itr = ValueItrAt(beatInfo.bpm, static_cast<Pulse>(pulseDouble));
	const Pulse nearestBPMChangePulse = itr->first;
	const double nearestBPM = itr->second;

	// Calculate sec using pulse difference from nearest tempo change
	return cache.bpmChangeSec.at(nearestBPMChangePulse) + (pulseDouble - static_cast<double>(nearestBPMChangePulse)) / kResolution * 60 / nearestBPM;
}

kson::Pulse kson::MsToPulse(double ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return SecToPulse(ms / 1000, beatInfo, cache);
}

kson::Pulse kson::SecToPulse(double sec, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest tempo change
	assert(!cache.bpmChangePulse.empty());
	const auto itr = ValueItrAt(cache.bpmChangePulse, sec);
	const double nearestBPMChangeSec = itr->first;
	const Pulse nearestBPMChangePulse = itr->second;
	const double nearestBPM = beatInfo.bpm.at(nearestBPMChangePulse);

	// Calculate pulse using time difference from nearest tempo change
	const Pulse pulse = nearestBPMChangePulse + static_cast<Pulse>(kResolution * (sec - nearestBPMChangeSec) * nearestBPM / 60);

	return pulse;
}

double kson::MsToPulseDouble(double ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return SecToPulseDouble(ms / 1000, beatInfo, cache);
}

double kson::SecToPulseDouble(double sec, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest tempo change
	assert(!cache.bpmChangePulse.empty());
	const auto itr = ValueItrAt(cache.bpmChangePulse, sec);
	const double nearestBPMChangeSec = itr->first;
	const Pulse nearestBPMChangePulse = itr->second;
	const double nearestBPM = beatInfo.bpm.at(nearestBPMChangePulse);

	// Calculate pulse using time difference from nearest tempo change
	const double pulseDouble = static_cast<double>(nearestBPMChangePulse) + kResolution * (sec - nearestBPMChangeSec) * nearestBPM / 60;

	return pulseDouble;
}

std::int64_t kson::PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangeMeasureIdx.empty());
	const auto itr = ValueItrAt(cache.timeSigChangeMeasureIdx, pulse);
	const Pulse nearestTimeSigChangePulse = itr->first;
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
	const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

	// Calculate measure count using time difference from nearest time signature change
	const std::int64_t measureCount = nearestTimeSigChangeMeasureIdx + static_cast<std::int64_t>((pulse - nearestTimeSigChangePulse) / TimeSigOneMeasurePulse(nearestTimeSig));

	return measureCount;
}

std::int64_t kson::MsToMeasureIdx(double ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return SecToMeasureIdx(ms / 1000, beatInfo, cache);
}

std::int64_t kson::SecToMeasureIdx(double sec, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return PulseToMeasureIdx(SecToPulse(sec, beatInfo, cache), beatInfo, cache);
}

kson::Pulse kson::MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangePulse.empty());
	const auto itr = ValueItrAt(cache.timeSigChangePulse, measureIdx);
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->first;
	const Pulse nearestTimeSigChangePulse = itr->second;
	const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

	// Calculate pulse using measure count difference from nearest time signature change
	const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureIdx - nearestTimeSigChangeMeasureIdx) * TimeSigOneMeasurePulse(nearestTimeSig));

	return pulse;
}

kson::Pulse kson::MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangePulse.empty());
	const std::int64_t measureIdx = static_cast<std::int64_t>(measureValue);
	const auto itr = ValueItrAt(cache.timeSigChangePulse, measureIdx);
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->first;
	const Pulse nearestTimeSigChangePulse = itr->second;
	const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

	// Calculate pulse using measure count difference from nearest time signature change
	const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureValue - nearestTimeSigChangeMeasureIdx) * TimeSigOneMeasurePulse(nearestTimeSig));

	return pulse;
}

double kson::MeasureValueToPulseDouble(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangePulse.empty());
	const std::int64_t measureIdx = static_cast<std::int64_t>(measureValue);
	const auto itr = ValueItrAt(cache.timeSigChangePulse, measureIdx);
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->first;
	const Pulse nearestTimeSigChangePulse = itr->second;
	const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

	// Calculate pulse using measure count difference from nearest time signature change
	const double pulseDouble = nearestTimeSigChangePulse + (measureValue - static_cast<double>(nearestTimeSigChangeMeasureIdx)) * TimeSigOneMeasurePulse(nearestTimeSig);

	return pulseDouble;
}

double kson::MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return MeasureIdxToSec(measureIdx, beatInfo, cache) * 1000;
}

double kson::MeasureIdxToSec(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return PulseToSec(MeasureIdxToPulse(measureIdx, beatInfo, cache), beatInfo, cache);
}

double kson::MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return MeasureValueToSec(measureValue, beatInfo, cache) * 1000;
}

double kson::MeasureValueToSec(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
	return PulseToSec(MeasureValueToPulse(measureValue, beatInfo, cache), beatInfo, cache);
}

bool kson::IsBarLinePulse(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangeMeasureIdx.empty());
	const auto itr = ValueItrAt(cache.timeSigChangeMeasureIdx, pulse);
	const Pulse nearestTimeSigChangePulse = itr->first;
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
	const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

	return ((pulse - nearestTimeSigChangePulse) % TimeSigOneMeasurePulse(nearestTimeSig)) == 0;
}

double kson::TempoAt(Pulse pulse, const BeatInfo& beatInfo)
{
	// Fetch the nearest BPM change
	assert(!beatInfo.bpm.empty());
	return ValueItrAt(beatInfo.bpm, pulse)->second;
}

const kson::TimeSig& kson::TimeSigAt(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
	// Fetch the nearest time signature change
	assert(!cache.timeSigChangeMeasureIdx.empty());
	const auto itr = ValueItrAt(cache.timeSigChangeMeasureIdx, pulse);
	const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
	return beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);
}

kson::Pulse kson::LastNoteEndY(const kson::NoteInfo& noteInfo)
{
	auto maxPulse = kson::Pulse{ 0 };
	for (const auto& lane : noteInfo.bt)
	{
		const kson::Pulse y = LastNoteEndYButtonLane(lane);
		if (y > maxPulse)
		{
			maxPulse = y;
		}
	}
	for (const auto& lane : noteInfo.fx)
	{
		const kson::Pulse y = LastNoteEndYButtonLane(lane);
		if (y > maxPulse)
		{
			maxPulse = y;
		}
	}
	for (const auto& lane : noteInfo.laser)
	{
		const kson::Pulse y = LastNoteEndYLaserLane(lane);
		if (y > maxPulse)
		{
			maxPulse = y;
		}
	}
	return maxPulse;
}

kson::Pulse kson::LastNoteEndYButtonLane(const kson::ByPulse<kson::Interval>& lane)
{
	if (lane.empty())
	{
		return kson::Pulse{ 0 };
	}

	const auto& [y, lastNote] = *lane.rbegin();

	return y + lastNote.length;
}

kson::Pulse kson::LastNoteEndYLaserLane(const kson::ByPulse<kson::LaserSection>& lane)
{
	if (lane.empty())
	{
		return kson::Pulse{ 0 };
	}

	const auto& [y, lastSection] = *lane.rbegin();
	if (lastSection.v.empty())
	{
		assert(false && "Laser section must not be empty");
		return y;
	}

	const auto& [ry, lastPoint] = *lastSection.v.rbegin();
	return y + ry;
}

double kson::GetModeBPM(const BeatInfo& beatInfo)
{
	constexpr double kErrorBPM = 120.0;

	if (beatInfo.bpm.empty())
	{
		assert(false && "beatInfo.bpm is empty");
		return kErrorBPM;
	}

	if (beatInfo.bpm.size() == 1U)
	{
		return beatInfo.bpm.begin()->second;
	}

	// Calculate total pulse duration for each BPM value
	std::unordered_map<std::int32_t, RelPulse> bpmTotalPulses;
	Pulse prevY = Pulse{ 0 };
	std::optional<std::int32_t> prevBPMInt;
	for (const auto& [y, bpm] : beatInfo.bpm)
	{
		if (y < prevY)
		{
			assert(false && "y must be larger than or equal to prevY in beatInfo.bpm");
			return kErrorBPM;
		}
		if (prevBPMInt.has_value())
		{
			bpmTotalPulses[prevBPMInt.value()] += y - prevY;
		}

		prevY = y;
		prevBPMInt = static_cast<std::int32_t>(bpm);
	}

	if (bpmTotalPulses.empty())
	{
		// Only one BPM change exists
		if (prevBPMInt.has_value())
		{
			return static_cast<double>(prevBPMInt.value());
		}
		assert(false && "bpmTotalPulses must not be empty");
		return kErrorBPM;
	}

	// Find BPM with largest total pulse duration
	const auto itr = std::max_element(
		bpmTotalPulses.begin(),
		bpmTotalPulses.end(),
		[](const auto& a, const auto& b) { return a.second < b.second; });

	if (itr == bpmTotalPulses.end())
	{
		assert(false && "max_element must not return end iterator");
		return kErrorBPM;
	}

	const auto& [modeBPM, modeBPMTotalPulse] = *itr;
	return static_cast<double>(modeBPM);
}

double kson::GetEffectiveStdBPM(const ChartData& chartData)
{
	return chartData.meta.stdBPM > 0.0 ? chartData.meta.stdBPM : GetModeBPM(chartData.beat);
}
