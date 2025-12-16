module;

#include "kson/Common/Common.hpp"

export module kson:Common;

export namespace kson {
    using kson::kNumBTLanes;
    using kson::kNumFXLanes;
    using kson::kNumLaserLanes;
    using kson::kNumBTLanesSZ;
    using kson::kNumFXLanesSZ;
    using kson::kNumLaserLanesSZ;
    using kson::Pulse;
    using kson::RelPulse;
    using kson::kResolution;
    using kson::kResolution4;
    using kson::kCurveSubdivisionInterval;
    using kson::ByPulse;
    using kson::BTLane;
    using kson::FXLane;
    using kson::LaserLane;
    using kson::ByPulseMulti;
    using kson::ByRelPulse;
    using kson::ByRelPulseMulti;
    using kson::ByMeasureIdx;
    using kson::DefKeyValuePair;
    using kson::GraphValue;
    using kson::GraphCurveValue;
    using kson::GraphPoint;
    using kson::Interval;
    using kson::Graph;
    using kson::GraphSection;
    using kson::Dict;
    using kson::ValueItrAt;
    using kson::ValueAtOrDefault;
    using kson::CountInRange;
    using kson::FirstInRange;
    using kson::RemoveFloatingPointError;
    using kson::AlmostEquals;
}
