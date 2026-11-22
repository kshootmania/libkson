#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct CamGraphs
	{
		Graph zoomBottom; // "zoom_bottom"
		Graph zoomSide; // "zoom_side"
		Graph zoomTop; // "zoom_top"
		Graph rotationDeg; // Rotation degree
		Graph rotationDegHighway; // "rotation_deg_highway"
		Graph rotationDegJdgline; // "rotation_deg_jdgline"
		Graph rotationDegLayer; // "rotation_deg_layer"
		Graph centerSplit; // "center_split"
		Graph highwayWipe; // "highway_wipe"
	};

	struct CamPatternInvokeSpinValue
	{
		std::int32_t count = 1;
	};

	struct CamPatternInvokeHalfSpinValue
	{
		double deg = 60.0;
	};

	struct CamPatternInvokeSwingValue
	{
		double scale = 250.0;
		std::int32_t repeat = 3;
		std::int32_t decayOrder = 2;
	};

	struct CamPatternInvokeCustomValue
	{
		double scale = 1.0;
	};

	namespace detail
	{
		template <typename ValueType>
		struct BasicCamPatternInvoke
		{
			std::int32_t d = 0; // laser slam direction, -1 (left) or 1 (right)
			RelPulse length = 0;
			ValueType v = {};
		};
	}

	using CamPatternInvokeSpin = detail::BasicCamPatternInvoke<CamPatternInvokeSpinValue>;
	using CamPatternInvokeHalfSpin = detail::BasicCamPatternInvoke<CamPatternInvokeHalfSpinValue>;
	using CamPatternInvokeSwing = detail::BasicCamPatternInvoke<CamPatternInvokeSwingValue>;
	using CamPatternInvokeCustom = detail::BasicCamPatternInvoke<CamPatternInvokeCustomValue>;

	struct CamPatternLaserInvokeList
	{
		ByPulse<CamPatternInvokeSpin> spin;
		ByPulse<CamPatternInvokeHalfSpin> halfSpin;
		ByPulse<CamPatternInvokeSwing> swing;
		Dict<ByPulseMulti<CamPatternInvokeCustom>> custom; // Key: name defined in CamPatternInfo::def
	};

	struct CamPatternLaserInfo
	{
		CamPatternLaserInvokeList slamEvent;
	};

	struct CamPatternInvokePulse
	{
		RelPulse length = 0;
		CamPatternInvokeCustomValue v = {};
	};

	struct CamPatternDef
	{
		CamGraphs body; // Keys of the graphs are relative pulses from the beginning of the pattern

		bool slamShake = true; // Whether the default shake of a laser slam is applied together with the pattern

		bool flipByDirection = true; // Whether the values are flipped for a laser slam to the left

		// Returns the largest relative pulse of the graph points
		[[nodiscard]]
		RelPulse length() const;
	};

	using CamPatternDefKVP = DefKeyValuePair<CamPatternDef>;

	// Returns true if the name is reserved for built-in cam patterns ("spin", "half_spin", "swing")
	[[nodiscard]]
	bool IsBuiltInCamPatternName(std::string_view name);

	// Returns false if the name cannot be used as a cam pattern name in KSH (empty, built-in, or containing separators)
	[[nodiscard]]
	bool IsValidKshCamPatternName(std::string_view name);

	struct CamPatternInfo
	{
		std::vector<CamPatternDefKVP> def;
		CamPatternLaserInfo laser;
		Dict<ByPulse<CamPatternInvokePulse>> pulseEvent; // Key: name defined in def

		// Returns the definition with the specified name, or nullptr if not defined
		[[nodiscard]]
		const CamPatternDef* defByName(std::string_view name) const;
	};

	struct CamInfo
	{
		CamGraphs body;
		CamPatternInfo pattern;
	};
}
