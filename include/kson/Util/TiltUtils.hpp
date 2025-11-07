#pragma once
#include <optional>
#include "kson/Common/Common.hpp"
#include "kson/Camera/Tilt.hpp"

namespace kson
{
	// Get manual tilt value at the current pulse
	std::optional<double> ManualTiltValueAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse);

	// Get auto tilt scale at the current pulse
	double AutoTiltScaleAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse);

	// Get auto tilt keep enabled at the current pulse
	bool AutoTiltKeepAt(const ByPulse<TiltValue>& tiltValue, Pulse currentPulse);
}
