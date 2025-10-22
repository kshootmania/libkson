#include "kson/util/graph_utils.hpp"
#include "kson/util/graph_curve.hpp"

double kson::GraphValueAt(const Graph& graph, Pulse pulse)
{
	if (graph.empty())
	{
		return 0.0;
	}

	auto itr = graph.upper_bound(pulse);
	if (itr == graph.end())
	{
		return graph.rbegin()->second.v.vf;
	}
	if (itr != graph.begin())
	{
		--itr;
	}

	const auto& [pulse1, point1] = *itr;
	if (pulse < pulse1)
	{
		return point1.v.v;
	}

	const auto nextItr = std::next(itr);
	if (nextItr == graph.end())
	{
		return point1.v.vf;
	}

	const auto& [pulse2, point2] = *nextItr;
	assert(pulse1 <= pulse && pulse < pulse2);

	const double lerpRate = static_cast<double>(pulse - pulse1) / static_cast<double>(pulse2 - pulse1);

	// Apply curve if present
	const double curveValue = EvaluateCurve(point1.curve, lerpRate);

	return std::lerp(point1.v.vf, point2.v.v, curveValue);
}
