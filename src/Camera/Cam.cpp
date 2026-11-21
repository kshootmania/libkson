#include "kson/Camera/Cam.hpp"
#include <algorithm>

kson::RelPulse kson::CamPatternDef::length() const
{
	RelPulse length = 0;
	for (const Graph* pGraph : { &body.zoomBottom, &body.zoomSide, &body.zoomTop, &body.rotationDeg, &body.rotationDegHighway, &body.rotationDegJdgline, &body.rotationDegLayer, &body.centerSplit, &body.highwayWipe })
	{
		if (!pGraph->empty())
		{
			length = std::max(length, pGraph->rbegin()->first);
		}
	}
	return length;
}

bool kson::IsBuiltInCamPatternName(std::string_view name)
{
	return name == "spin" || name == "half_spin" || name == "swing";
}

bool kson::IsValidKshCamPatternName(std::string_view name)
{
	return !name.empty() && !IsBuiltInCamPatternName(name) && name.find_first_of(" \t;\r\n") == std::string_view::npos;
}

const kson::CamPatternDef* kson::CamPatternInfo::defByName(std::string_view name) const
{
	for (const auto& [defName, def] : def)
	{
		if (defName == name)
		{
			return &def;
		}
	}
	return nullptr;
}
