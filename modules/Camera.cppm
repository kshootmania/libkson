module;

#include "kson/Camera/Cam.hpp"
#include "kson/Camera/CameraInfo.hpp"
#include "kson/Camera/Tilt.hpp"

export module kson:Camera;

export namespace kson {
    using kson::CamGraphs;
    using kson::CamPatternInvokeSwingValue;
    using kson::CamPatternInvokeSpin;
    using kson::CamPatternInvokeSwing;
    using kson::CamPatternLaserInvokeList;
    using kson::CamPatternLaserInfo;
    using kson::CamPatternInfo;
    using kson::CamInfo;
    using kson::CameraInfo;
    using kson::AutoTiltType;
    using kson::TiltGraphValue;
    using kson::TiltGraphPoint;
    using kson::TiltValue;
    using kson::GetAutoTiltScale;
    using kson::IsKeepAutoTiltType;
}
