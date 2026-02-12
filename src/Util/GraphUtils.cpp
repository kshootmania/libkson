#include "kson/Util/GraphUtils.hpp"
#include "kson/Util/GraphCurve.hpp"
#include <algorithm>
#include <vector>

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

	const Pulse segmentLength = pulse2 - pulse1;
	if (segmentLength <= 0)
	{
		return point2.v.v;
	}

	const double lerpRate = static_cast<double>(pulse - pulse1) / static_cast<double>(segmentLength);

	// Apply curve if present
	const double curveValue = EvaluateCurve(point1.curve, lerpRate);

	return std::lerp(point1.v.vf, point2.v.v, curveValue);
}

kson::Graph kson::BakeStopIntoScrollSpeed(const Graph& scrollSpeed, const ByPulse<RelPulse>& stop)
{
	if (stop.empty())
	{
		return scrollSpeed;
	}

	Graph baseScrollSpeed = scrollSpeed;
	if (baseScrollSpeed.empty())
	{
		baseScrollSpeed.emplace(0, GraphValue{ 1.0 });
	}

	std::vector<std::pair<Pulse, Pulse>> mergedStopRanges;
	for (const auto& [stopY, stopLength] : stop)
	{
		const Pulse start = stopY;
		const Pulse end = stopY + stopLength;

		if (mergedStopRanges.empty() || mergedStopRanges.back().second < start)
		{
			mergedStopRanges.emplace_back(start, end);
		}
		else
		{
			mergedStopRanges.back().second = std::max(mergedStopRanges.back().second, end);
		}
	}

	Graph result = baseScrollSpeed;

	for (const auto& [stopStart, stopEnd] : mergedStopRanges)
	{
		const double speedBeforeStop = GraphValueAt(result, stopStart);
		const double speedAfterStop = GraphValueAt(result, stopEnd);

		auto it = result.upper_bound(stopStart);
		while (it != result.end() && it->first < stopEnd)
		{
			it = result.erase(it);
		}

		result.insert_or_assign(stopStart, GraphValue{ speedBeforeStop, 0.0 });
		result.insert_or_assign(stopEnd, GraphValue{ 0.0, speedAfterStop });
	}

	return result;
}
