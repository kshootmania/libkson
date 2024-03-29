﻿#include "kson/util/graph_utils.hpp"

double kson::GraphValueAt(const Graph& graph, Pulse pulse)
{
	if (graph.empty())
	{
		return 0.0;
	}

	auto itr = graph.upper_bound(pulse);
	if (itr == graph.end())
	{
		return graph.rbegin()->second.vf;
	}
	if (itr != graph.begin())
	{
		--itr;
	}

	const auto& [pulse1, value1] = *itr;
	if (pulse < pulse1)
	{
		return value1.v;
	}

	const auto nextItr = std::next(itr);
	if (nextItr == graph.end())
	{
		return value1.vf;
	}

	const auto& [pulse2, value2] = *nextItr;
	assert(pulse1 <= pulse && pulse < pulse2);

	const double lerpRate = static_cast<double>(pulse - pulse1) / static_cast<double>(pulse2 - pulse1);
	return std::lerp(value1.vf, value2.v, lerpRate);
}
