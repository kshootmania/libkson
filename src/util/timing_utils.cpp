#include "kson/util/timing_utils.hpp"
#include <cassert>

kson::Pulse kson::TimeSigMeasurePulse(const TimeSig& timeSig)
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

    // The first tempo change should be placed at 0.0 ms
    cache.bpmChangeMs.emplace(0, 0.0);
    cache.bpmChangePulse.emplace(0.0, 0);

    // The first time signature change should be placed at 0.0 ms
    cache.timeSigChangePulse.emplace(0, 0);
    cache.timeSigChangeMeasureIdx.emplace(0, 0);

    // Calculate ms for each tempo change
    {
        double ms = 0.0;
        auto prevItr = beatInfo.bpm.cbegin();
        for (auto itr = std::next(prevItr); itr != beatInfo.bpm.cend(); ++itr)
        {
            ms += static_cast<double>(itr->first - prevItr->first) / kResolution * 60 * 1000 / prevItr->second;
            cache.bpmChangeMs[itr->first] = ms;
            cache.bpmChangePulse[ms] = itr->first;
            prevItr = itr;
        }
    }

    // Calculate measure count for each time signature change
    {
        Pulse pulse = 0;
        auto prevItr = beatInfo.timeSig.cbegin();
        for (auto itr = std::next(prevItr); itr != beatInfo.timeSig.cend(); ++itr)
        {
            pulse += (itr->first - prevItr->first) * (kResolution4 * prevItr->second.n / prevItr->second.d);
            cache.timeSigChangePulse[itr->first] = pulse;
            cache.timeSigChangeMeasureIdx[pulse] = itr->first;
            prevItr = itr;
        }
    }

    return cache;
}

kson::double kson::PulseToMs(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest BPM change
    assert(!beatInfo.bpm.empty());
    const auto itr = CurrentAt(beatInfo.bpm, pulse);
    const Pulse nearestBPMChangePulse = itr->first;
    const double nearestBPM = itr->second;

    // Calculate ms using pulse difference from nearest tempo change
    const double ms = cache.bpmChangeMs.at(nearestBPMChangePulse) + static_cast<double>(pulse - nearestBPMChangePulse) / kResolution * 60 * 1000 / nearestBPM;

    return ms;
}

double kson::PulseToSec(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMs(pulse, beatInfo, cache) / 1000;
}

kson::Pulse kson::MsToPulse(double ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest tempo change
    assert(!cache.bpmChangePulse.empty());
    const auto itr = CurrentAt(cache.bpmChangePulse, ms);
    const double nearestBPMChangeMs = itr->first;
    const Pulse nearestBPMChangePulse = itr->second;
    const double nearestBPM = beatInfo.bpm.at(nearestBPMChangePulse);

    // Calculate pulse using time difference from nearest tempo change
    const Pulse pulse = nearestBPMChangePulse + static_cast<Pulse>(kResolution * (ms - nearestBPMChangeMs) * nearestBPM / 60 / 1000);

    return pulse;
}

kson::Pulse kson::SecToPulse(double sec, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return MsToPulse(sec * 1000, beatInfo, cache);
}

std::int64_t kson::PulseToMeasureIdx(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    assert(!cache.timeSigChangeMeasureIdx.empty());
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure count using time difference from nearest time signature change
    const std::int64_t measureCount = nearestTimeSigChangeMeasureIdx + static_cast<std::int64_t>((pulse - nearestTimeSigChangePulse) / TimeSigMeasurePulse(nearestTimeSig));

    return measureCount;
}

std::int64_t kson::MsToMeasureIdx(double ms, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMeasureIdx(MsToPulse(ms, beatInfo, cache), beatInfo, cache);
}

std::int64_t kson::SecToMeasureIdx(double sec, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return MsToMeasureIdx(sec * 1000, beatInfo, cache);
}

kson::Pulse kson::MeasureIdxToPulse(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    assert(!cache.timeSigChangePulse.empty());
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const std::int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureIdx - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig));

    return pulse;
}

kson::Pulse kson::MeasureValueToPulse(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    assert(!cache.timeSigChangePulse.empty());
    const std::int64_t measureIdx = static_cast<std::int64_t>(measureValue);
    const auto itr = CurrentAt(cache.timeSigChangePulse, measureIdx);
    const Pulse nearestTimeSigChangeMeasureIdx = itr->first;
    const std::int64_t nearestTimeSigChangePulse = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    // Calculate measure using measure count difference from nearest time signature change
    const Pulse pulse = nearestTimeSigChangePulse + static_cast<Pulse>((measureValue - nearestTimeSigChangeMeasureIdx) * TimeSigMeasurePulse(nearestTimeSig));

    return pulse;
}

kson::double kson::MeasureIdxToMs(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMs(MeasureIdxToPulse(measureIdx, beatInfo, cache), beatInfo, cache);
}

double kson::MeasureIdxToSec(std::int64_t measureIdx, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return MeasureIdxToMs(measureIdx, beatInfo, cache) / 1000;
}

kson::double kson::MeasureValueToMs(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return PulseToMs(MeasureValueToPulse(measureValue, beatInfo, cache), beatInfo, cache);
}

double kson::MeasureValueToSec(double measureValue, const BeatInfo& beatInfo, const TimingCache& cache)
{
    return MeasureValueToMs(measureValue, beatInfo, cache) / 1000;
}

bool kson::IsPulseBarLine(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    assert(!cache.timeSigChangeMeasureIdx.empty());
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const Pulse nearestTimeSigChangePulse = itr->first;
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    const TimeSig& nearestTimeSig = beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);

    return ((pulse - nearestTimeSigChangePulse) % TimeSigMeasurePulse(nearestTimeSig)) == 0;
}

double kson::PulseTempo(Pulse pulse, const BeatInfo& beatInfo)
{
    // Fetch the nearest BPM change
    assert(!beatInfo.bpm.empty());
    return CurrentAt(beatInfo.bpm, pulse)->second;
}

const kson::TimeSig& kson::PulseTimeSig(Pulse pulse, const BeatInfo& beatInfo, const TimingCache& cache)
{
    // Fetch the nearest time signature change
    assert(!cache.timeSigChangeMeasureIdx.empty());
    const auto itr = CurrentAt(cache.timeSigChangeMeasureIdx, pulse);
    const std::int64_t nearestTimeSigChangeMeasureIdx = itr->second;
    return beatInfo.timeSig.at(nearestTimeSigChangeMeasureIdx);
}
