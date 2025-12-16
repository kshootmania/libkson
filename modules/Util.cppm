module;

#include "kson/Util/GraphCurve.hpp"
#include "kson/Util/GraphUtils.hpp"
#include "kson/Util/TiltUtils.hpp"
#include "kson/Util/TimingUtils.hpp"

export module kson:Util;

export namespace kson {
    using kson::EvaluateCurve;
    using kson::ExpandCurveSegments;
    using kson::GraphValueAt;
    using kson::BakeStopIntoScrollSpeed;
    using kson::GraphSectionAt;
    using kson::GraphSectionValueAt;
    using kson::GraphSectionValueAtWithDefault;
    using kson::GraphPointAt;
    using kson::ManualTiltValueAt;
    using kson::AutoTiltScaleAt;
    using kson::AutoTiltKeepAt;
    using kson::TimingCache;
    using kson::TimeSigOneMeasurePulse;
    using kson::CreateTimingCache;
    using kson::PulseToMs;
    using kson::PulseToSec;
    using kson::PulseDoubleToMs;
    using kson::PulseDoubleToSec;
    using kson::MsToPulse;
    using kson::SecToPulse;
    using kson::MsToPulseDouble;
    using kson::SecToPulseDouble;
    using kson::PulseToMeasureIdx;
    using kson::MsToMeasureIdx;
    using kson::SecToMeasureIdx;
    using kson::MeasureIdxToPulse;
    using kson::MeasureValueToPulse;
    using kson::MeasureValueToPulseDouble;
    using kson::MeasureIdxToMs;
    using kson::MeasureIdxToSec;
    using kson::MeasureValueToMs;
    using kson::MeasureValueToSec;
    using kson::IsBarLinePulse;
    using kson::TempoAt;
    using kson::TimeSigAt;
    using kson::GetModeBPM;
    using kson::GetEffectiveStdBPM;
    using kson::LastNoteEndY;
    using kson::LastNoteEndYButtonLane;
    using kson::LastNoteEndYLaserLane;
}
